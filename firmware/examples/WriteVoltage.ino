/*
  WriteVoltage
  
  Reads an analog voltage from pin 0, and writes it to a channel on ThingSpeak every 20 seconds.
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize and analyze live data streams in the cloud.
  
  Copyright 2017, The MathWorks, Inc.

  Documentation for the ThingSpeak Communication Library for Particle is in the doc folder where the library was installed.
  See the accompaning licence file for licensing information.
*/

#include "ThingSpeak/ThingSpeak.h"

TCPClient client;

/*
  *****************************************************************************************
  **** Visit https://www.thingspeak.com to sign up for a free account and create
  **** a channel.  The video tutorial http://community.thingspeak.com/tutorials/thingspeak-channels/ 
  **** has more information. You need to change this to your channel, and your write API key
  **** IF YOU SHARE YOUR CODE WITH OTHERS, MAKE SURE YOU REMOVE YOUR WRITE API KEY!!
  *****************************************************************************************/
unsigned long myChannelNumber = 31461;
const char * myWriteAPIKey = "LD79EOAAWRVYF04Y";

void setup() {
  ThingSpeak.begin(client);
}

void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Convert the analog reading 
  // On Particle: 0 - 4095 maps to 0 - 3.3 volts
  float voltage = sensorValue * (3.3 / 4095.0);

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  ThingSpeak.writeField(myChannelNumber, 1, voltage, myWriteAPIKey);
  delay(20000); // ThingSpeak will only accept updates every 15 seconds.
}
