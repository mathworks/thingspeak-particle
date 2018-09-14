/*
  WriteMultipleVoltages
  
  Reads analog voltages from pins 0-7 and writes them to the 8 fields of a channel on ThingSpeak every 20 seconds.
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize and analyze live data streams in the cloud.
  
  Copyright 2017, The MathWorks, Inc.

  Documentation for the ThingSpeak Communication Library for Particle is in the README.md file where the library was installed.
  See the accompanying license file for licensing information.
*/

#include "ThingSpeak.h"

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
  // Read the input on each pin, convert the reading, and set each field to be sent to ThingSpeak.
  // On Particle: 0 - 4095 maps to 0 - 3.3 volts
  float pinVoltage = analogRead(A0) * (3.3 / 4095.0);

  ThingSpeak.setField(1,pinVoltage);
  pinVoltage = analogRead(A1) * (3.3 / 4095.0);
  ThingSpeak.setField(2,pinVoltage);
  pinVoltage = analogRead(A2) * (3.3 / 4095.0);
  ThingSpeak.setField(3,pinVoltage);
  pinVoltage = analogRead(A3) * (3.3 / 4095.0);
  ThingSpeak.setField(4,pinVoltage);
  pinVoltage = analogRead(A4) * (3.3 / 4095.0);
  ThingSpeak.setField(5,pinVoltage);
  pinVoltage = analogRead(A5) * (3.3 / 4095.0);
  ThingSpeak.setField(6,pinVoltage);
  pinVoltage = analogRead(A6) * (3.3 / 4095.0);
  ThingSpeak.setField(7,pinVoltage);
  pinVoltage = analogRead(A7) * (3.3 / 4095.0);
  ThingSpeak.setField(8,pinVoltage);
  
  // Write the fields that you've set all at once.
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  

  delay(20000); // ThingSpeak will only accept updates every 15 seconds. 
}
