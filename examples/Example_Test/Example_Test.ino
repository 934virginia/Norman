//digitalClockLog and logDigits functions modified from example sketches included in RTCLib library created by Adafruit.
//All other functions and methods created by 934Virginia, 2019-07-08, and released into the public domain.

#include <time.h>

#include <Dusk2Dawn.h>
#include "Norman.h"

//Outputs a formatted year's worth of data for graphical comparison to real-world data. 
//Just copy-paste the content of the serial monitor to a .csv, and you're good to go.

void digitalClockLog(time_t Xtime) {
  // digital clock display of the time
  //Formatted to match NCDC timestamps for climatic normals.
  
  //logDigits(getYear(Xtime));
  //Serial.print('-');
  logDigits(getMonth(Xtime));
  Serial.print('-');
  logDigits(getDay(Xtime));
  Serial.print("T");
  logDigits(getHour(Xtime));
  Serial.print(':');
  logDigits(getMinute(Xtime));
  Serial.print(':');
  logDigits(getSecond(Xtime));
}

void logDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

////////////////////////////////////STRUCT INPUT////////////////////////////////////////////////////////////////
//SetTarget_Origin Structure is (Latitude, Longitude, Timezone(standard time), Season Start, Season Length).
//Season Length is unused for Origin parameters.

//SetRangeTDP Structure is (coldest day of year, hottest day of year, Minimum value on coldest day, Maximum value on coldest day, Minimum value on hottest day, Maximum value on hottest day).
//if you can't find minimum and maximum Dew Point values for your location on the hottest and coldest days of the year, 
//you'll need to use the RHtoDP(Temp, RH, "F") function for the temperature and RH data at those points in time. 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Denver
struct SetTarget_Origin Target = {39.7392, -104.9903, -7, getTime(2010, 1, 1, 0, 0, 0), 365};
struct SetTarget_Origin Origin = {39.7392, -104.9903, -7, getTime(2010, 1, 1, 0, 0, 0), 365};
struct SetRangeTDP Temperature = {getTime(2010, 12, 30, 0, 0, 0), getTime(2010, 7, 10, 0, 0, 0), 24.8, 39.0, 60.8, 85.1};
struct SetRangeTDP DewPoint = {getTime(2010, 12, 30, 0, 0, 0), getTime(2010, 7, 10, 0, 0, 0), 12.9, 16.7, 42.2, 48.6};


time_t currentTime = getTime(2010, 1, 1, 0, 0, 0);


void setup() {
  Serial.begin(9600);
  Serial.print("\"");
  Serial.print("Dateime");
  Serial.print("\",\"");
  Serial.print("Temperature F");
  Serial.print("\",\"");
  Serial.print("Dewpoint F");
  Serial.print("\",\"");
  Serial.print("Percent Relative Humidity");
  Serial.println("\""); 
  
}

void loop() {

  currentTime += 3600;

  /*
  //If you want to print whether or not the sun is up, uncomment this section. This messes up the formatting for comparisons, though.
  time_t testoRise = getSun(currentTime, Target, Origin, "Rise");
  time_t testoSet = getSun(currentTime, Target, Origin, "Set");

  if (currentTime >= testoRise && currentTime < testoSet) {
    Serial.println("DAY");
  }
  else {
    Serial.println("NIGHT");
  }
  */

  //Get your projections
  float testoT = getTDP(currentTime, Target, Origin, Temperature, 150);
  float testoDP = getTDP(currentTime, Target, Origin, DewPoint, 150);
  float testoRH = getRH(testoT, testoDP, "F");
  //float testoRH = getRH(testoT, testoDP, "C");

  //Silly way to ensure it stops printing the values after a year.
  if (getYear(currentTime) == 2010) 
  {
    Serial.print("\"");
    digitalClockLog(currentTime);
    Serial.print("\",\"");
    Serial.print(testoT);
    Serial.print("\",\"");
    Serial.print(testoDP);
    Serial.print("\",\"");
    Serial.print(testoRH);
    Serial.println("\""); 
  }

}
