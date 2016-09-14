/*
  CheerLights
  
  Reads the latest CheerLights color on ThingSpeak, and sets a common anode RGB LED on digital pins 5, 6, and 9.
  On Spark core, the built in RGB LED is used
  Visit http://www.cheerlights.com for more info.

  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize and analyze live data streams in the cloud.
  
  Copyright 2016, The MathWorks, Inc.

  Documentation for the ThingSpeak Communication Library for Particle is in the doc folder where the library was installed.
  See the accompaning licence file for licensing information.
*/

#include "ThingSpeak/ThingSpeak.h"

// Make sure that you put a 330 ohm resistor between the Particle
// pins and each of the color pins on the LED.
int pinRed = 9;
int pinGreen = 6;
int pinBlue = 5;

TCPClient client;


/*
  This is the ThingSpeak channel number for CheerLights
  https://thingspeak.com/channels/1417.  Field 1 contains a string with
  the latest CheerLights color.
*/
unsigned long cheerLightsChannelNumber = 1417;

void setup() {
  ThingSpeak.begin(client);
}

void loop() {
  // Read the latest value from field 1 of channel 1417
  String color = ThingSpeak.readStringField(cheerLightsChannelNumber, 1);
  setColor(color);

  // Check again in 5 seconds
  delay(5000);
}

// List of CheerLights color names
String colorName[] = {"none","red","pink","green","blue","cyan","white","warmwhite","oldlace","purple","magenta","yellow","orange"};

// Map of RGB values for each of the Cheerlight color names
int colorRGB[][3] = {     0,  0,  0,  // "none"
                        255,  0,  0,  // "red"
                        255,192,203,  // "pink"
                          0,255,  0,  // "green"
                          0,  0,255,  // "blue"
                          0,255,255,  // "cyan",
                        255,255,255,  // "white",
                        255,223,223,  // "warmwhite",
                        255,223,223,  // "oldlace",
                        128,  0,128,  // "purple",
                        255,  0,255,  // "magenta",
                        255,255,  0,  // "yellow",
                        255,165,  0}; // "orange"};

void setColor(String color)
{
  // Look through the list of colors to find the one that was requested
  for(int iColor = 0; iColor <= 12; iColor++)
  {
    if(color == colorName[iColor])
    {
      // When it matches, look up the RGB values for that color in the table,
      // and write the red, green, and blue values.

	  RGB.control(true);
	  RGB.color(colorRGB[iColor][0], colorRGB[iColor][1], colorRGB[iColor][2]);

      return;
    }
  }
}
