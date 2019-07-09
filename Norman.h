/*	Norman.h - Library for climatic simulation.
  	Created by 934Virginia, 2019-07-08.
	Released into the public domain.
	Dusk2Dawn library, NOAA Solar Calculator, and DayofYear function are the property of their respective authors.
*/
#ifndef Norman_h
#define Norman_h

	#include "Arduino.h"
	#include "Dusk2Dawn.h"
	#include <time.h>

	struct SetTarget_Origin {
		float Lat;
		float Lon;
		int TZ;
		time_t StartDT;
		int SLength;
	};
	struct SetRangeTDP {
		time_t xminL;
		time_t xmaxL;
		float minlow;
		float minhigh;
		float maxlow;
		float maxhigh;
	};	


	//Norman();
	//struct SetTarget_Origin(float, float, int, time_t, int);
	//struct SetRangeTDP(time_t, time_t, float, float, float, float);
	time_t getTime(int, byte, byte, byte, byte, byte);
	bool DSTCheck(time_t);
	float RHtoDP(float, float, String);
	time_t getSun(time_t, struct SetTarget_Origin, struct SetTarget_Origin, String);
	float getTDP(time_t, struct SetTarget_Origin, struct SetTarget_Origin, struct SetRangeTDP, int);
	float getRH(float, float, String);



	int getYear(time_t);
	byte getMonth(time_t);
	byte getDay(time_t);
	byte getHour(time_t);
	byte getMinute(time_t);
	byte getSecond(time_t);
	float cosTDP(float, float, float, float, float);
	float polyTDP(float, float, float, float, float);
	int DayofYear(int, int, int);
	time_t halfTime(time_t, time_t);
	time_t getRiseSet(float, float, int, float, float, int, time_t, time_t, int, int, String, int);
	float baseD(float, float, float, float, time_t, time_t, time_t, time_t, int, long, int, int, float, int);
	float baseN(float, float, float, float, time_t, time_t, time_t, time_t, int, long, int, int, time_t, int);
	long getTPeak(time_t, time_t, int);
	int getDOYRange(int, int, time_t);
	time_t calcSun(time_t, float, float, int, float, float, int, time_t, time_t, int, String);
	float calcTDP(time_t, float, float, int, float, float, int, time_t, time_t, int, time_t, time_t, float, float, float, float, int);
		
#endif
