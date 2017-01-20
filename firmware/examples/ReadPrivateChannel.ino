/*
  ReadPrivateChannel
  
  Reads the latest voltage value from a private channel on ThingSpeak every 30 seconds
  and prints to the serial port debug window.

  For an example of how to read from a public channel, see ReadChannel example
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize and analyze live data streams in the cloud.
  
  Copyright 2017, The MathWorks, Inc.

  Documentation for the ThingSpeak Communication Library for Particle is in the doc folder where the library was installed.
  See the accompaning licence file for licensing information.
*/

#include "ThingSpeak/ThingSpeak.h"

// Particle Core, Photon, and Electron the results are published to the Particle dashboard using events.
// Go to http://dashboard.particle.io, click on the logs tab, and you'll see the events coming in. 
 TCPClient client;

/*
  *****************************************************************************************
  **** Visit https://www.thingspeak.com to sign up for a free account and create
  **** a channel.  The video tutorial http://community.thingspeak.com/tutorials/thingspeak-channels/ 
  **** has more information. You need to change this to your channel, and your read API key
  **** IF YOU SHARE YOUR CODE WITH OTHERS, MAKE SURE YOU REMOVE YOUR READ API KEY!!
  *****************************************************************************************

  This is the ThingSpeak channel used in the write examples (31461).  It is private, and requires a
  read API key to access it.  We'll read from the first field.
*/
unsigned long myChannelNumber = 31461;
const char * myReadAPIKey = "NKX4Z5JGO4M5I18A";

void setup() {
  ThingSpeak.begin(client);
}

void loop() {
  // Read the latest value from field 1 of channel 31461
  float voltage = ThingSpeak.readFloatField(myChannelNumber, 1, myReadAPIKey);

  Particle.publish("thingspeak-readvoltage", "Latest voltage is: " + String(voltage) + "V",60,PRIVATE);
  delay(30000);
}
