/*
  ReadLastTemperature
  
  Reads the latest temperature from the MathWorks weather station in Natick, MA
  https://thingspeak.com/channels/12397 on ThingSpeak, and prints to
  the serial port debug window every 30 seconds.
  
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
  https://thingspeak.com/channels/12397.  It senses a number of things, including 
  Wind Direction, Wind Speed, Humidity, Temperature, Rainfall, and Atmospheric Pressure.

  Temperature is stored in field 4
*/

unsigned long weatherStationChannelNumber = 12397;
unsigned int temperatureFieldNumber = 4;

void setup() {  
  ThingSpeak.begin(client);
}

void loop() {
  // Read the latest value from field 4 of channel 12397
  float temperatureInF = ThingSpeak.readFloatField(weatherStationChannelNumber, temperatureFieldNumber);

  Particle.publish("thingspeak-lasttemp", String::format("Current temp %.1f degrees F",temperatureInF),60,PRIVATE);
  delay(30000); // Note that the weather station only updates once a minute
}
