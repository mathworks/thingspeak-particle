/*
  ReadWeatherStation
  
  Reads the latest weather data every 60 seconds from the public MathWorks
  weather station in Natick, MA https://thingspeak.com/channels/12397 on ThingSpeak.
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize and analyze live data streams in the cloud.
  
  Copyright 2016, The MathWorks, Inc.

  Documentation for the ThingSpeak Communication Library for Particle is in the doc folder where the library was installed.
  See the accompaning licence file for licensing information.
*/

#include "ThingSpeak/ThingSpeak.h"

// On Particle Core, Photon, and Electron the results are published to the Particle dashboard using events.
// Go to http://dashboard.particle.io, click on the logs tab, and you'll see the events coming in. 
TCPClient client;

/*
  This is the ThingSpeak channel number for the MathwWorks weather station
  https://thingspeak.com/channels/12397.  It senses a number of things and puts them in the eight
  field of the channel:
  Field 1 - Wind Direction (degrees where 0 is North)
  Field 2 - Wind Speed (MPH)
  Field 3 - Humidity (%RH)
  Field 4 - Temperature (Degrees F)
  Field 5 - Rainfall (inches since last measurement)
  Field 6 - Atmospheric Pressure (inHg)
*/
unsigned long weatherStationChannelNumber = 12397;

void setup() { 
  ThingSpeak.begin(client);
}

void loop() {
  float windDirection = ThingSpeak.readFloatField(weatherStationChannelNumber,1);
  float windSpeed = ThingSpeak.readFloatField(weatherStationChannelNumber,2);
  float humidity = ThingSpeak.readFloatField(weatherStationChannelNumber,3);
  float temperature = ThingSpeak.readFloatField(weatherStationChannelNumber,4);
  float rainfall = ThingSpeak.readFloatField(weatherStationChannelNumber,5);
  float pressure = ThingSpeak.readFloatField(weatherStationChannelNumber,6);

  Particle.publish("thingspeak-weather", "Current weather conditions in Natick: ",60,PRIVATE);
  Particle.publish("thingspeak-weather", String(temperature) + " degrees F, " + String(humidity) + "% humidity",60,PRIVATE); 
  Particle.publish("thingspeak-weather", "Wind at " + String(windSpeed) + " MPH at " + String (windDirection) + " degrees",60,PRIVATE); 
    if(rainfall > 0)
	{
	  Particle.publish("thingspeak-weather", "Pressure is " + String(pressure) + " inHg, and it's raining",60,PRIVATE);
	}
	else
	{
	  Particle.publish("thingspeak-weather", "Pressure is " + String(pressure) + " inHg",60,PRIVATE);
	}

  delay(60000); // Note that the weather station only updates once a minute

}
