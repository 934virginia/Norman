# Norman

Arduino Library for mathematical climatic simulation of natural daylight, temperature, and humidity cycles using minimal data sets.

Given target coordinates, specified date ranges, and minimum/maximum values for temperature and humidity at target's annual extremes, generate time_t values for sunrise and sunset, and floating point numbers for temperature, dew point, and relative humidity. Requires [Dusk2Dawn library by DM Kishi](https://github.com/dmkishi/Dusk2Dawn); an adapted C++ port of the [NOAA Solar Calculator](https://www.esrl.noaa.gov/gmd/grad/solcalc/).

## Licensing
Raw climatic data used in this project, as well as original Solar Calculator software provided by NOAA ESRL Global Monitoring Division, Boulder, Colorado, USA (http://esrl.noaa.gov/gmd/) 

All included libraries and dependencies are the property of their respective authors.

## Installation
### From the ZIP file
1. Download the [ZIP file](https://github.com/934Virginia/Norman/archive/master.zip).
2. Launch the Arduino IDE and navigate to *Sketch → Include Library → Add .ZIP Library...*. From the prompt, select the ZIP just downloaded.

## Usage
Included example scripts can be utilized to either project formatted data, or control climatic conditions using RTCLib, Adafruit_Sensor, DHT, and DHT_U libraries created by Adafruit. 
Example datasets for various US cities included in parameters.txt file. Just copy-paste them into the Example_Test script to get annual data.

## FAQ
### What is this?

This is Norman. Norman is a climatic simulation library that mathematically projects natural cycles of daylight, temperature, and humidity for remote locations (and periods of time) using minimal data sets. It was designed to handle climate control for indoor/greenhouse agriculture and botany applications when paired with an RTC module. It was built in the Arduino IDE and tested on an ATmega328P. It was named for Norman Borlaug; agronomist of legend and friend to all who enjoy not starving to death.

### How does it work?
#### Daylight
Norman makes extensive use of the Dusk2Dawn library, by DM Kishi. It relies on Dusk2Dawn to retrieve daily times for sunrise and sunset based on latitude, longitude, and timezone of the target location. Optionally, it anchors these daily light cycles to solar noon of local daylight schedules. This enables users utilizing a greenhouse to incorporate photoresistors and PWM driver control for the purposes of energy-efficient supplemental lighting if so desired.

#### Temperature and Humidity
Yearly temperature and dew point trends can be loosely expressed by a sine wave in most climates. This means we can project daily minima and maxima as long as we have minimum and maximum values for the hottest and coldest days of the year, as well as the indexed locations of these days for the target location. Seasonal lag was loosely accounted for by modifying the width of this sine wave based on the number of days between the hottest and coldest days of the year (relative to the current time). Daily patterns for temperature and dew point were more complicated. I used a polynomial function for daytime shifts, an exponential function for nightly decay, and a regressive anchoring system in order to create smooth transitions between day and night cycles while continuing to follow the annual sine wave. Relative humidity is retrieved using these temperature and dew point projections. This enables users to incorporate these projected values into climate control applications using popular temperature and humidity sensors (DHT11, DHT22, BMP280, etc.).

### Does it only calculate values for the current day?
No. It can be programmed to mimic conditions for any time of the year, from any time of the year. Optionally, users can designate the length and starting point of a "season," and repeat the projected conditions of this time period over and over again without having to re-flash the sketch. This method retains the (optional) functionality of anchoring target daylight cycles to local cycles happening in real time.

### How accurate is it?
That's a complicated question. You can see for yourself at [my plot.ly page](https://plot.ly/~934Virginia). I tested a dozen US cities to see how my projections would stack up, and I think it did alright, considering how crude it is. Seems to do a lot better in coastal and continental climates than mountains and deserts. I've got some ideas of how to improve its consistency, but addressing those issues would necessitate the time and labor of people who actually know what they're doing. As it is, I'm ready to move on to other projects, so we're calling it as done as done gets.

### Why are you writing all this?
Because I hope that my dumb little project will inflame the curiosity of smart people. I want other people to do this better, because I think a comprehensive mathematical model that can accurately simulate remote climatic conditions might have some exciting implications for agricultural development and botany. If you are interested in taking a swing at it, contact me. I can put you onto some questions and observations that might help you get started. Probably the first order of business would be whipping up some sunrise and sunset functions that build and fade naturally based on solar elevation and azimuth. It would also be rad if someone sent me a picture of them growing passion fruit in Siberia using my garbage code and a $10 microcontroller.

## Known Bugs
(Under construction!)

## History
- **2019-7-8**: Released.
