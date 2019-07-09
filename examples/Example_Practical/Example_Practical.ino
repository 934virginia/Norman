// All included libraries and related code snippets extracted and modified from example sketches included in said libraries are the property of their respective authors.
//getStartDTO and LTime functions created by 934Virginia, 2019-07-08, and released into the public domain.

#include <time.h>
#include "RTClib.h"
#include <Dusk2Dawn.h>
#include "Norman.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN            2         // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)


DHT_Unified dht(DHTPIN, DHTTYPE);

//Type of RTC being used. I highly recommend the DS3231 for long-term accuracy.
RTC_DS3231 rtc;


//Define pins responsible for controlling specific relays. Can define rf codes similarly if going that route.//
int Lights = 7;

int TempPlus = 8;
int TempMinus = 9;
int RHPlus = 10;
int RHMinus = 11;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Declare how close to keep real-world climate conditions collected by the sensor to our mathematically projected values 
//(Target range = projected value +- this integer.)
//The smaller the integer, the more precisely your devices will attempt to control your environment.
int Precision = 2;

//Conversion to make RTC DT values play nice with our calculations
time_t LTime(DateTime x)  {
  int yearx = x.year();
  byte monthx = x.month();
  byte dayx = x.day();
  byte hourx = x.hour();
  byte minutex = x.minute();
  byte secondx = x.second();

  struct tm tmSet;
  tmSet.tm_year = yearx - 1900;
  tmSet.tm_mon = monthx - 1;
  tmSet.tm_mday = dayx;
  tmSet.tm_hour = hourx;
  tmSet.tm_min = minutex;
  tmSet.tm_sec = secondx;
  return mk_gmtime(&tmSet) + UNIX_OFFSET;
}

void digitalClockLog(time_t Xtime) {
  // digital clock display of the time
  logDigits(getYear(Xtime));
  Serial.print('-');
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

//Sets the start date of the first "season" to the date we flashed the sketch, and adjusts for DST. 
//If you flash between midnight and 1am during DST, expect the season to start the day before.
time_t getStartDTO()  {
  if (DSTCheck(LTime(DateTime(F(__DATE__), F(__TIME__)))) == true) {
    time_t StartDTOadj = LTime(DateTime(F(__DATE__), F(__TIME__)) - TimeSpan(0, 1, 0, 0));
    return getTime(getYear(StartDTOadj), getMonth(StartDTOadj), getDay(StartDTOadj), 0, 0, 0);
  }
  else  {
    time_t StartDTOx = LTime(DateTime(F(__DATE__), F(__TIME__)));
    return getTime(getYear(StartDTOx), getMonth(StartDTOx), getDay(StartDTOx), 0, 0, 0);
  }
}
time_t StartDTO = getStartDTO();


////////////////////////////////////STRUCT INPUT////////////////////////////////////////////////////////////////
//SetTarget_Origin Structure = (Latitude, Longitude, Timezone(standard time), Season Start, Season Length).
//Season Length is unused for Origin parameters.

//SetRangeTDP Structure = (coldest day of year, hottest day of year, Minimum value on coldest day, Maximum value on coldest day, Minimum value on hottest day, Maximum value on hottest day).
//if you can't find minimum and maximum Dew Point values for your location on the hottest and coldest days of the year, 
//you'll need to use the RHtoDP(Temp, RH, "F") function for the temperature and RH data at those points in time. 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//!Origin Season Start MUST be changed from copied parameter value to StartDTO in order to sync with RTC!

//Changing Origin Lat, Lon, and TZ values to your own local values will center projected sunrise and sunset times on your own solar noon.
//Use this to tinker with supplemental lighting setups, or skip entering your local values altogether.

//////////////////////////////////////////////////////

//San Diego//
struct SetTarget_Origin Target = {32.7157, -117.1611, -8, getTime(2019, 4, 15, 0, 0, 0), 90};
struct SetTarget_Origin Origin = {32.7157, -117.1611, -8, StartDTO, 365};
struct SetRangeTDP Temperature = {getTime(2010, 12, 28, 0, 0, 0), getTime(2010, 8, 26, 0, 0, 0), 49.9, 63.2, 67.8, 75.9};
struct SetRangeTDP DewPoint = {getTime(2010, 12, 28, 0, 0, 0), getTime(2010, 8, 26, 0, 0, 0), 39.6, 46.5, 62.5, 63.5};

//////////////////////////////////////////////////////


time_t currentTime;

void setup() {
  Serial.begin(9600);
  Serial.println("--------------------------------------------------");

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    //The following sets the RTC to the date & time this sketch was compiled; runs a check to see if it was flashed during DST, and adjusts it to standard time if necessary. 
    //If you live somewhere like Arizona or in a country with unique DST rules, you'll need to tinker with this, along with getStartDTO function.
    DateTime rtcDT;
    if (DSTCheck(LTime(DateTime(F(__DATE__), F(__TIME__)))) == true) {
      rtcDT = DateTime(F(__DATE__), F(__TIME__)) - TimeSpan(0, 1, 0, 0);
    }
    rtc.adjust(rtcDT);
  }

  // Initialize sensor.
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);

  //Ready up your relay output pins.
  pinMode(Lights, OUTPUT);
  pinMode(TempPlus, OUTPUT);
  pinMode(TempMinus, OUTPUT);
  pinMode(RHPlus, OUTPUT);
  pinMode(RHMinus, OUTPUT);

}

void loop() {

  
  currentTime = LTime(rtc.now());

  sensors_event_t event;
  dht.temperature().getEvent(&event);

  float TempReal = (float(event.temperature) * 1.8 + 32);
  //Use this instead if measuring in Celsius.
  //int TempReal = int(event.temperature);

  dht.humidity().getEvent(&event);
  float RHReal = float(event.relative_humidity);


  time_t projRise = getSun(currentTime, Target, Origin, "Rise");
  time_t projSet = getSun(currentTime, Target, Origin, "Set");

  Serial.print("Current Time: ");
  digitalClockLog(currentTime);
  Serial.println();

  Serial.print("Sunrise: ");
  digitalClockLog(projRise);
  Serial.println();
  Serial.print("Sunset: ");
  digitalClockLog(projSet);
  Serial.println();

  
  if (currentTime >= projRise && currentTime < projSet) {
    Serial.println("Lights ON");
    digitalWrite(Lights, HIGH);
  }
  else {
    Serial.println("Lights OFF");
    digitalWrite(Lights, LOW);
  }


  //Structure is Current Time, Target location values (SetTarget_Origin), Origin location values (SetTarget_Origin), Numerical parameters (SetRangeTDP), Daily thermal peak (hottest time of the day, expressed in minutes after solar noon).
  //Thermal peak is a really weird piece of this project. 150 will work fine for most projections, but I built in explicit front-end access so that people can fiddle with it and plug in a proper algorithm. 
  //TL;DR: Leave it at 150 unless you're a solar physicist.
  float TempProj = getTDP(currentTime, Target, Origin, Temperature, 150);
  float DPProj = getTDP(currentTime, Target, Origin, DewPoint, 150);
  float RHProj = getRH(TempProj, DPProj, "F");
  //float RHProj = getRH(TempProj, DPProj, "C");

  Serial.print("Actual temperature): ");
  Serial.print(TempReal);
  Serial.println(" F");
  //Serial.println(" C");
  
  Serial.print("Projected temperature: ");
  Serial.print(TempProj);
  Serial.println(" F");
  //Serial.println(" C");


  if (TempReal > (TempProj + Precision))
  {
    digitalWrite(TempMinus, HIGH);
    digitalWrite(TempPlus, LOW);
    Serial.println("High temperature! Cooling environment.");
  }
  else if (TempReal < (TempProj - Precision))
  {
    digitalWrite(TempPlus, HIGH);
    digitalWrite(TempMinus, LOW);
    Serial.println("Low temperature! Heating environment.");
  }

  Serial.print("Actual RH: ");
  Serial.print(RHReal);
  Serial.println("%");
  
  Serial.print("Projected RH: ");
  Serial.print(RHProj);
  Serial.println("%");

  if (RHReal > (RHProj + Precision))
  {
    digitalWrite(RHMinus, HIGH);
    digitalWrite(RHPlus, LOW);
    Serial.println("High humidity! Dehumidifying environment.");
  }
  else if (RHReal < (RHProj - Precision))
  {
    digitalWrite(RHPlus, HIGH);
    digitalWrite(RHMinus, LOW);
    Serial.println("Low humidity! Humidifying environment.");
  }
  Serial.println("--------------------------------------------------");

  //Adjust this delay to control frequency of readings and control operations. 
  //Another option is to use TimeAlarms, but that gets fiddly with avr time functions.
  delay(10000);
}
