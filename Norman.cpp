#include "Arduino.h"
#include <time.h>
#include "Norman.h"
#include "Dusk2Dawn.h"






//Unused as of now, but included in case someone needs to interact with syncprovider
/*
time_t syncProvider() {
  DateTime x = rtc.now();
  
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
*/



//creates time_t object
time_t getTime(int yearx, byte monthx, byte dayx, byte hourx, byte minutex, byte secondx)  {
  
  struct tm tmSet;
  tmSet.tm_year = yearx - 1900;
  tmSet.tm_mon = monthx - 1;
  tmSet.tm_mday = dayx;
  tmSet.tm_hour = hourx;
  tmSet.tm_min = minutex;
  tmSet.tm_sec = secondx;
  return mk_gmtime(&tmSet) + UNIX_OFFSET;
}

int getYear(time_t timeIn) {
  timeIn -= UNIX_OFFSET;
  struct tm * getStruct;
  getStruct = gmtime(&timeIn);
  
  return (getStruct->tm_year) + 1900;
}

byte getMonth(time_t timeIn) {
  timeIn -= UNIX_OFFSET;
  struct tm * getStruct;
  getStruct = gmtime(&timeIn);
  
  return (getStruct->tm_mon) + 1;
}

byte getDay(time_t timeIn) {
  timeIn -= UNIX_OFFSET;
  struct tm * getStruct;
  getStruct = gmtime(&timeIn);
  
  return (getStruct->tm_mday);
}

byte getHour(time_t timeIn) {
  timeIn -= UNIX_OFFSET;
  struct tm * getStruct;
  getStruct = gmtime(&timeIn);
  
  return (getStruct->tm_hour);
}

byte getMinute(time_t timeIn) {
  timeIn -= UNIX_OFFSET;
  struct tm * getStruct;
  getStruct = gmtime(&timeIn);
  
  return (getStruct->tm_min);
}

byte getSecond(time_t timeIn) {
  timeIn -= UNIX_OFFSET;
  struct tm * getStruct;
  getStruct = gmtime(&timeIn);
  
  return (getStruct->tm_sec);
}

//used to get daily lows and highs from lows and highs of hottest and coldest days of the year
float cosTDP(float x, float ymin, float ymax, float xmin, float cycle) {
  return (((ymax + ymin) / 2) - (((ymax - ymin) / 2) * cos((((2 * PI) / cycle) * x) - (((2 * PI) / cycle) * xmin))));
}

//our base function for the daytime temperature and dewpoint shifts
float polyTDP(float x, float ymin, float ymax, float xmin, float cycle) {
  return ((pow((x - xmin), 2)) * ((1 / (pow((cycle / 2), 4) / (ymax - ymin))) * (pow((x - cycle), 2))) + ymin);  
}

//alternate daytime function-- the curvature is a bit closer to raw data, but there's no exponential ramp up from dawn doing it this way. Also throws baseN off. 
//worth playing with if you want to rework things.
/*float paraTDP(float x, float ymin, float ymax, float xmin, float cycle) {
  return (ymax - (((ymax - ymin) / (cycle / 2)) / (cycle / 2)) * pow((((x - xmin) - (cycle / 2))), 2));
}*/


//"toDayOfYear.ino" function swiped from jrleeman. Returns numerical day of calendar year.
int DayofYear(int dayx, int monthx, int yearx) {
  
  int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  
  // Verify we got a 4-digit year
  if (yearx < 1000 || dayx < 1 || dayx > daysInMonth[monthx-1]) {
    return 999;
  }
  
  if (yearx%4  == 0) {
    if (yearx%100 != 0) {
      daysInMonth[1] = 29;
    }
    else {
      if (yearx%400 == 0) {
        daysInMonth[1] = 29;
      }
    }
   }
  
  int doy = 0;
  for (int i = 0; i < monthx - 1; i++) {
    doy += daysInMonth[i];
  }
  
  doy += dayx;
  return doy;
}


//Some of this stuff is probably super redundant.


time_t halfTime(time_t one, time_t two) {
  if (two > one)  {
    return one + (long (two - one) / 2);
  }
  else  {
    return two + (long (one - two) / 2);
  }
}


//Quick and dirty DST check for flash time.
bool DSTCheck(time_t flash)  {
  int DSTy = 8;
  int DSTx = 13 - ((((getYear(flash) - 2016) / 4) * 2) + ((getYear(flash) - 2016) - ((getYear(flash) - 2016) / 4)));
  if (DSTx >= 8)  {
    DSTy = DSTx;
  }
  else if (DSTx == 1) {
    DSTy = (DSTx + 7);
  }
  else  {
    DSTy = (((((8 - DSTx) / 7) + 1) * 7) + DSTx);
  }
  if (flash < getTime(getYear(flash), 11, (DSTy - 7), 2, 0, 0) || flash >= getTime(getYear(flash), 3, DSTy, 2, 0, 0)) {
    return true;
  }
  else  {
    return false;
  }
}

//jenkball method to anchor target daylight hours to Origin hours, centered on Origin's solar noon. If Target = Origin, it does nothing.
time_t getRiseSet(float tLat, float tLon, int tTZ, float oLat, float oLon, int oTZ, time_t startDTTar, time_t startDTOrig, int SPosition, int ODiff, String RiseOrSet, int adj)  {

  Dusk2Dawn target(tLat, tLon, tTZ);
  Dusk2Dawn origin(oLat, oLon, oTZ);

  startDTTar = startDTTar + ((SPosition + adj) * 86400L);
  startDTOrig = startDTOrig + ((ODiff + adj) * 86400L);

  int targetSunrise = target.sunrise(getYear(startDTTar), getMonth(startDTTar), getDay(startDTTar), false);
  int targetSunset = target.sunset(getYear(startDTTar), getMonth(startDTTar), getDay(startDTTar), false);

  int originSunrise = origin.sunrise(getYear(startDTOrig), getMonth(startDTOrig), getDay(startDTOrig), false);
  int originSunset = origin.sunset(getYear(startDTOrig), getMonth(startDTOrig), getDay(startDTOrig), false);

  time_t LStartTar = getTime(getYear(startDTTar), getMonth(startDTTar), getDay(startDTTar), int((targetSunrise) / 60), int((targetSunrise) % 60), 0);
  time_t LEndTar = getTime(getYear(startDTTar), getMonth(startDTTar), getDay(startDTTar), int((targetSunset) / 60), int((targetSunset) % 60), 0);
  time_t LStartOrig = getTime(getYear(startDTOrig), getMonth(startDTOrig), getDay(startDTOrig), int((originSunrise) / 60), int((originSunrise) % 60), 0);
  time_t LEndOrig = getTime(getYear(startDTOrig), getMonth(startDTOrig), getDay(startDTOrig), int((originSunset) / 60), int((originSunset) % 60), 0);

  if (RiseOrSet == "Rise") {
    return (halfTime(LEndOrig, LStartOrig)) - (difftime(LEndTar, LStartTar) / 2);
  }
  else if (RiseOrSet == "Set") {
    return (halfTime(LEndOrig, LStartOrig)) + (difftime(LEndTar, LStartTar) / 2);
  }

}

//The giant, disgusting basic function of daily temperature and humidity cycles. Sunrise to TPeak is anchored to the previous night's baseN value at sunrise. 
//TPeak to sunset is bound to the cosTDP-derived daily maximum value.
float baseD(float minlow, float maxlow, float minhigh, float maxhigh, time_t currentTime, time_t LStart, time_t LEnd, time_t StartDT, int SPositionx, long TPeak, int xmin, int xmax, float baseNMinus, int adj) {
  
  int SPosition = SPositionx;
  int currentTimex = DayofYear(getDay(currentTime), getMonth(currentTime), getYear(currentTime));
  int xminx = xmin;
  time_t StartDTadj = StartDT + ((SPosition + adj) * 86400L);
  int TarTimex = DayofYear(getDay(StartDTadj), getMonth(StartDTadj), getYear(StartDTadj));
  
    if (xmax > xmin) {
      if (TarTimex >= xmax && xmax < 354)  {
        xminx += 365;
      }
      else if (xmax >= 354)  {
        if  (TarTimex >= xmax)  {
          xminx += 365;
        }    
        else if (TarTimex < xmin)  {
          xminx += 0;
        }
      }
    }
    
    else if (xmin > xmax)  {
      if (TarTimex >= xmin && xmin < 354) {
        xminx -= 365;
      }
      else if (xmin >= 354)  {
        if  (TarTimex >= xmin)  {
          xminx += 0;
        }
        else if (TarTimex < xmax)  {
          xminx -= 365;
        }
      }
    }
    
  int AdjDOY = DayofYear(getDay(StartDTadj), getMonth(StartDTadj), getYear(StartDTadj));
  if (AdjDOY >= 365 && DayofYear(getDay(StartDT), getMonth(StartDT), getYear(StartDT)) <= 1) {
    if (getYear(StartDTadj) < getYear(StartDT)) {
      AdjDOY -= 365;
    }
  }

  float ymin = cosTDP(AdjDOY, minlow, maxlow, xminx, (getDOYRange(xmin, xmax, StartDTadj)));

  if (currentTime < (LStart + TPeak))  {
    ymin = baseNMinus;
  }

  float currentX = (((currentTime - LStart) / 3600) + (float((currentTime - LStart) % 3600) / 3600));
  float ymax = cosTDP(AdjDOY, minhigh, maxhigh, xminx, (getDOYRange(xmin, xmax, StartDTadj)));
  float cycleX = ((((LStart + TPeak) - LStart) / 3600) + (float(((LStart + TPeak) - LStart) % 3600) / 3600)) * 2;

  return polyTDP(currentX, ymin, ymax, 0, cycleX);  
  //return paraTDP(currentX, ymin, ymax, 0, cycleX);
}

//Calculations for nighttime temperature and humidity. Starting point derived regressively from BaseD values at sunset.
float baseN(float minlow, float maxlow, float minhigh, float maxhigh, time_t currentTime, time_t LStart, time_t LEnd, time_t StartDT, int SPosition, long TPeak, int xmin, int xmax, time_t LStartN, int adj) {
  int SPosS, SPosE = 0;
  time_t StartDTadj = StartDT + ((SPosition + adj)* 86400L); 
  int AdjDOY = DayofYear(getDay(StartDTadj), getMonth(StartDTadj), getYear(StartDTadj));
  if (AdjDOY >= 365 && DayofYear(getDay(StartDT), getMonth(StartDT), getYear(StartDT)) <= 1) {    
    if (getYear(StartDTadj) < getYear(StartDT)) {
      AdjDOY -= 365;
    }
  }
  float x = ((currentTime - LEnd) / 3600) + (float((currentTime - LEnd) % 3600) / 3600);
  int currentTimex = DayofYear(getDay(currentTime), getMonth(currentTime), getYear(currentTime));
  int xminx = xmin;
  int TarTimex = DayofYear(getDay(StartDTadj), getMonth(StartDTadj), getYear(StartDTadj));


  
  if (xmax > xmin) {
    if (TarTimex >= xmax && xmax < 354)  {
      xminx += 365;
    }
    else if (xmax >= 354)  {
      if  (TarTimex >= xmax)  {
        xminx += 365;
      }    
      else if (TarTimex < xmin)  {
        xminx += 0;
      }
    }
  }
  
  else if (xmin > xmax)  {
    if (TarTimex >= xmin && xmin < 354) {
      xminx -= 365;
    }
    else if (xmin >= 354)  {
      if  (TarTimex >= xmin)  {
        xminx += 0;
      }
      else if (TarTimex < xmax)  {
        xminx -= 365;
      }
    }
  }
  
  if ((getDay(LStart) != getDay(LEnd))) {
    if (getDay(currentTime) == getDay(LEnd)) {
      SPosS += 1;
    }
    else if (getDay(currentTime) == getDay(LStart)) {
      SPosE-=1;
    }     
    
    float LEndVal = baseD(minlow, maxlow, minhigh, maxhigh, LEnd, LStartN, LEnd, StartDT, SPosition, TPeak, xminx, xmax, 0, (SPosE + adj));
    float LStartVal = cosTDP(AdjDOY, minlow, maxlow, xminx, (getDOYRange(xmin, xmax, StartDTadj)));

    //the .8 constant wasn't really derived from any legit calculations, it was just the product of a lot of fiddling to see what came closest. Tinker with it as you see fit.
    return ((LEndVal - LStartVal) * (pow(.8, x))) + LStartVal;
  }
}
 

//Convert Relative Humidity to Dewpoint. Included because why not. Requires RH and Temperature at a given point in time.
//If you cant's get dewpoint in your weather data, but you have RH and Temperature at the low and high points of the hottest and coldest days, it should serve just fine.
float RHtoDP(float Temp, float RH, String CorF) {
  if (CorF == "F") {
    Temp -=  32;
    Temp /= 1.8;
    return ((243.04 * (log(RH / 100) + ((17.625 * Temp) / (243.04 + Temp))) / (17.625 - log(RH / 100) - ((17.625 * Temp)/(243.04 + Temp)))) * (1.8)) + 32;
  }
  else if (CorF == "C") {
    return 243.04 * (log(RH / 100) + ((17.625 * Temp) / (243.04 + Temp))) / (17.625 - log(RH / 100) - ((17.625 * Temp)/(243.04 + Temp)));
  }
}

//returns the number of seconds past dawn that thermal peak occurs. Necessary for getting cycle length in baseD.
long getTPeak(time_t LStart, time_t LEnd, int TPeakDelay) {
  return(difftime((halfTime(LEnd, LStart) + (TPeakDelay * 60)), LStart));
}

//returns number of days between hottest and coldest day of the year for whenever currentTime is relative to those days. 
//Creates a reasonable facsimile of seasonal lag without diving into complex calculations of solar irradiance.
int getDOYRange(int xmin, int xmax, time_t currentTimex) {
  int currentTime = (DayofYear(getDay(currentTimex), getMonth(currentTimex), getYear(currentTimex)));
  if (xmin < xmax)  {
    if ((xmin <= currentTime) && (currentTime < xmax)) {
      return (xmax - xmin) * 2;
    }
    else  {
      return ((xmin - xmax) + 365) * 2;
    }
  }
  else if (xmax < xmin)  {
    if ((xmax <= currentTime) && (currentTime < xmin)) {
      return (xmin - xmax) * 2;
    }
    else  {
      return ((xmax - xmin) + 365) * 2;
    }
  }
}

time_t calcSun(time_t currentTime, float tLat, float tLon, int tTZ, float oLat, float oLon, int oTZ, time_t startDTTar, time_t startDTOrig, int SLength, String RiseOrSet) {

  //Total time from season start and position within the seasonal loop, respectively//
  int ODiff = (difftime(currentTime, startDTOrig) / 86400L);
  int SPosition = ODiff % SLength;

  //sets adjusted sunrise and sunset values based on Target and Origin astronomical calculations
  time_t LStart = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, startDTTar, startDTOrig, SPosition, ODiff, "Rise", 0);
  time_t LEnd = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, startDTTar, startDTOrig, SPosition, ODiff, "Set", 0);  
  

  /////////////////Contextually adjusts sunrise and sunset and regressive values, sets TPeak accordingly/////////////////
  if (LStart > LEnd)  {
    if (currentTime >= LStart) {
      LEnd = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, startDTTar, startDTOrig, SPosition, ODiff, "Set", 1);
    }
    else if (currentTime < LEnd) {
      LStart = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, startDTTar, startDTOrig, SPosition, ODiff, "Rise", -1);
    }
  }
  else if (LEnd > LStart)  {
    if (currentTime < LStart) {
      LEnd = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, startDTTar, startDTOrig, SPosition, ODiff, "Set", -1);
    }
    else if (currentTime >= LEnd) {
      LStart = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, startDTTar, startDTOrig, SPosition, ODiff, "Rise", 1);

    }
  }

  if (RiseOrSet == "Rise") {
    return LStart;
  }
  else if (RiseOrSet == "Set")  {
    return LEnd;
  }
}

time_t getSun(time_t currentTime, struct SetTarget_Origin Tar, struct SetTarget_Origin Orig, String RiseOrSet)  {
  return calcSun(currentTime, Tar.Lat, Tar.Lon, Tar.TZ, Orig.Lat, Orig.Lon, Orig.TZ, Tar.StartDT, Orig.StartDT, Tar.SLength, RiseOrSet);
}

float calcTDP (time_t currentTime, float tLat, float tLon, int tTZ, float oLat, float oLon, int oTZ, time_t StartDT, time_t StartDTO, int SLength, time_t xminL, time_t xmaxL, float TDPminlow, float TDPminhigh, float TDPmaxlow, float TDPmaxhigh, int TPeakDelay) {

  int xmin = DayofYear(getDay(xminL), getMonth(xminL), getYear(xminL));
  int xmax = DayofYear(getDay(xmaxL), getMonth(xmaxL), getYear(xmaxL));

  //Total time from season start and position within the seasonal loop, respectively//
  int ODiff = (difftime(currentTime, StartDTO) / 86400L);
  int SPosition = ODiff % SLength;

  //sets adjusted sunrise and sunset values based on Target and Origin astronomical calculations
  time_t LStart = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Rise", 0);
  time_t LEnd = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Set", 0);
  //Sets regressive sunrise and sunset values
  time_t LStartN = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Rise", -1);
  time_t LEndN = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Set", -1);
  
  //Defines the thermal peak of a day's cycle based on start and end times; "TPeakDelay" is set to 150 minutes after solar noon by default on the front end in getTDP function; feel free to play with that based on local conditions
  long TPeak = getTPeak(LStart, LEnd, TPeakDelay);

  

  /////////////////Contextually adjusts sunrise and sunset and regressive values, sets TPeak accordingly/////////////////
  if (LStart > LEnd)  {
    if (currentTime >= LStart) {
      LEndN = LEnd;
      LEnd = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Set", 1);
      TPeak = getTPeak(LStart, LEnd, TPeakDelay);
    }
    else if (currentTime < LEnd) {
      LStart = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Rise", -1);
      LEndN = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Set", -1);
      TPeak = getTPeak(LStart, LEnd, TPeakDelay);
    }
  }
  else if (LEnd > LStart)  {
    if (currentTime < LStart) {
      LEnd = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Set", -1);
      TPeak = getTPeak((getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Rise", -1)), LEnd, TPeakDelay);
    }
    else if (currentTime >= LEnd) {
      LStartN = LStart;
      LStart = getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Rise", 1);
      LEndN = LEnd;
      TPeak = getTPeak((getRiseSet(tLat, tLon, tTZ, oLat, oLon, oTZ, StartDT, StartDTO, SPosition, ODiff, "Rise", 0)), LEnd, TPeakDelay);
    }
  }
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  //Daytime logic. If this is true, the sun is up. 
  if (LStart <= currentTime && currentTime < LEnd) {
    int SPositionx = SPosition - 1;
    
    if ((getDay(LStart) != getDay(LEnd)) && (getDay(currentTime) == getDay(LEnd))) {
      SPosition = SPositionx;
    }
   //Gets regressive projected values for temperature and dewpoint using baseN at dawn. Necessary for everything up to TPeak.
    float baseNMinusTDP = baseN(TDPminlow, TDPmaxlow, TDPminhigh, TDPmaxhigh, LStart, LStart, LEndN, StartDT, SPosition, TPeak, xmin, xmax, LStartN, -1);   
    return baseD(TDPminlow, TDPmaxlow, TDPminhigh, TDPmaxhigh, currentTime, LStart, LEnd, StartDT, SPosition, TPeak, xmin, xmax, baseNMinusTDP, 0); 
    }
  
  //nighttime logic. If this is true, the sun isn't up. No defining regressive values necessary here; built into baseN.
  else {
    return baseN(TDPminlow, TDPmaxlow, TDPminhigh, TDPmaxhigh, currentTime, LStart, LEnd, StartDT, SPosition, TPeak, xmin, xmax, LStartN, 0); 
  }  
}

float getTDP (time_t currentTime, struct SetTarget_Origin Tar, struct SetTarget_Origin Orig, struct SetRangeTDP Range, int TPeakDelay) {
  return calcTDP(currentTime, Tar.Lat, Tar.Lon, Tar.TZ, Orig.Lat, Orig.Lon, Orig.TZ, Tar.StartDT, Orig.StartDT, Tar.SLength, Range.xminL, Range.xmaxL, Range.minlow, Range.minhigh, Range.maxlow, Range.maxhigh, TPeakDelay);
}

//Convert Dewpoint and Temperature to Relative Humidity. Necessary for sensor readings. Requires Temperature, Dewpoint, and system of measurement.
float getRH(float Tempx, float DPx, String CorF) {
  float Temp = Tempx;
  float DP = DPx;
  if (CorF == "F") {
    Temp = (Tempx - 32) / 1.8;
    DP = (DPx - 32) / 1.8;
  }
  return 100 * (exp((17.625 * DP)/(243.04 + DP)) / exp((17.625 * Temp)/(243.04 + Temp)));
}
