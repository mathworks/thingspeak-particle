# ThingSpeak Communication Library for Particle

This library enables Particle hardware to write or read data to or from ThingSpeak, an open data platform for the Internet of Things with MATLAB analytics and visualization.

ThingSpeak offers free data storage and analysis of time-stamped numeric or alphanumeric data. Users can access ThingSpeak by visiting https://thingspeak.com and creating a ThingSpeak user account.

ThingSpeak stores data in channels. Channels support an unlimited number of timestamped observations (think of these as rows in a spreadsheet). Each channel has up to 8 fields (think of these as columns in a speadsheet). Check out this [video](http://www.mathworks.com/videos/introduction-to-thingspeak-107749.html) for an overview.

Channels may be public, where anyone can see the data, or private, where only the owner and select users can read the data. Each channel has an associated Write API Key that is used to control who can write to a channel. In addition, private channels have one or more Read API Keys to control who can read from private channel. An API Key is not required to read from public channels.  Each channel can have up to 8 fields. One field is created by default.

You can visualize and do online analytics of your data on ThingSpeak using the built-in version of MATLAB, or use the desktop version of MATLAB to get deeper historical insight. Visit https://www.mathworks.com/hardware-support/thingspeak.html to learn more.

#### Particle Web IDE
In the Particle Web IDE, click the libraries tab, find ThingSpeak, and choose "Include in App"

## Compatible Hardware:
* Particle (Formally Spark) Core, [Photon](https://www.particle.io/prototype#photon), [Electron](https://www.particle.io/prototype#electron) and [P1](https://www.particle.io/prototype#p0-and-p1).

# Some Quick Examples

## Write to a Channel Field
```
#include "ThingSpeak.h"

TCPClient client;

unsigned long myChannelNumber = 31461;	// change this to your channel number
const char * myWriteAPIKey = "LD79EOAAWRVYF04Y"; // change this to your channels write API key

void setup() {
	ThingSpeak.begin(client);
}

void loop() {
	// read the input on analog pin 0:
	int sensorValue = analogRead(A0);
	
	// Write to ThingSpeak, field 1, immediately
	ThingSpeak.writeField(myChannelNumber, 1, sensorValue, myWriteAPIKey);
	delay(20000); // ThingSpeak will only accept updates every 15 seconds.
}

```
## Write to a Multiple Channel fields at once
```
#include "ThingSpeak.h"

TCPClient client;

unsigned long myChannelNumber = 31461;	// change this to your channel number
const char * myWriteAPIKey = "LD79EOAAWRVYF04Y"; // change this to your channel write API key

void setup() {
	ThingSpeak.begin(client);
}

void loop(){
	// read the input on analog pins 1, 2 and 3:
	int sensorValue1 = analogRead(A1);
	int sensorValue2 = analogRead(A2);
	int sensorValue3 = analogRead(A3);
	
	// set fields one at a time
	ThingSpeak.setField(1,sensorValue1);
	ThingSpeak.setField(2,sensorValue2);
	ThingSpeak.setField(3,sensorValue3);
	
	// set the status if over the threshold
	if(sensorValue1 > 100){
		ThingSpeak.setStatus("ALERT! HIGH VALUE");
	}
	
	// Write the fields that you've set all at once.
	ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
	
	delay(20000); // ThingSpeak will only accept updates every 15 seconds.
}

```
## Read from a Public Channel
```
#include "ThingSpeak.h"

TCPClient client;

unsigned long weatherStationChannelNumber = 12397;

void setup() { 
  ThingSpeak.begin(client);
}

void loop(){
	
	// Read latest measurements from the weather station in Natick, MA
	float temperature = ThingSpeak.readFloatField(weatherStationChannelNumber,4);
	float humidity = ThingSpeak.readFloatField(weatherStationChannelNumber,3);
	
	Particle.publish("thingspeak-weather", "Current weather conditions in Natick: ",60,PRIVATE);
	Particle.publish("thingspeak-weather", String(temperature) + " degrees F, " + String(humidity) + "% humidity",60,PRIVATE); 
	
	delay(60000); // Note that the weather station only updates once a minute

}
```
## Read from a Private Channel
```
#include "ThingSpeak.h"

TCPClient client;

unsigned long myChannelNumber = 31461;
const char * myReadAPIKey = "NKX4Z5JGO4M5I18A";

void setup() { 
  ThingSpeak.begin(client);
}

void loop(){
	
	 // Read the latest value from field 1 of channel 31461
	float value = ThingSpeak.readFloatField(myChannelNumber, 1, myReadAPIKey);
	
	Particle.publish("thingspeak-value", "Latest value is: " + String(value),60,PRIVATE);
	delay(30000);

}
```

# Documentation

## begin
Initializes the ThingSpeak library and network settings.
```
bool begin (client) // defaults to ThingSpeak.com
```
```
bool begin (client, customHostName, port)
```
```
bool begin (client, customIP, port)
```
| Parameter      | Type         | Description                                            |          
|----------------|:-------------|:-------------------------------------------------------|
| client         | Client &     | TCPClient created earlier in the sketch                |
| customHostName | const char * | Host name of a custom install of ThingSpeak            |
| customIP       | IPAddress    | IP address of a custom install of ThingSpeak           |
| port           | unsigned int | Port number to use with a custom install of ThingSpeak |

### Returns
Always returns true. This does not validate the information passed in, or generate any calls to ThingSpeak.

## writeField
Write a value to a single field in a ThingSpeak channel.
```
int writeField(channelNumber, field, value, writeAPIKey)
```
| Parameter     | Type          | Description                                                                                     |          
|---------------|:--------------|:------------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                  |
| field         | unsigned int  | Field number (1-8) within the channel to write to.                                              |
| value         | int           | Integer value (from -32,768 to 32,767) to write.                                                |
|               | long          | Long value (from -2,147,483,648 to 2,147,483,647) to write.                                     |
|               | float         | Floating point value (from -999999000000 to 999999000000) to write.                             |
|               | String        | String to write (UTF8 string). ThingSpeak limits this field to 255 bytes.                       |
|               | const char *  | Character array (zero terminated) to write (UTF8). ThingSpeak limits this field to 255 bytes.   |
| writeAPIKey   | const char *  | Write API key associated with the channel. If you share code with others, do not share this key |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
Special characters will be automatically encoded by this method. See the note regarding special characters below.

## writeFields
Write a multi-field update. Call setField() for each of the fields you want to write first. 
```
int writeFields (channelNumber, writeAPIKey)	
```
| Parameter     | Type          | Description                                                                                     |          
|---------------|:--------------|:------------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                  |
| writeAPIKey   | const char *  | Write API key associated with the channel. If you share code with others, do not share this key |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
Special characters will be automatically encoded by this method. See the note regarding special characters below.

## writeRaw
Write a raw POST to a ThingSpeak channel. 
```
int writeRaw (channelNumber, postMessage, writeAPIKey)	
```

| Parameter     | Type          | Description                                                                                                                                       |          
|---------------|:--------------|:--------------------------------------------------------------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                                                                    |
| postMessage   | const char *  | Raw URL to write to ThingSpeak as a String. See the documentation at https://thingspeak.com/docs/channels#update_feed.                            |
|               | String        | Raw URL to write to ThingSpeak as a character array (zero terminated). See the documentation at https://thingspeak.com/docs/channels#update_feed. | 
| writeAPIKey   | const char *  | Write API key associated with the channel. If you share code with others, do not share this key                                                   |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
This method will not encode special characters in the post message.  Use '%XX' URL encoding to send special characters. See the note regarding special characters below.

## setField
Set the value of a single field that will be part of a multi-field update.
```
int setField (field, value)
```

| Parameter | Type         | Description                                                                                   |          
|-----------|:-------------|:----------------------------------------------------------------------------------------------|
| field     | unsigned int | Field number (1-8) within the channel to set                                                  |
| value     | int          | Integer value (from -32,768 to 32,767) to write.                                              |
|           | long         | Long value (from -2,147,483,648 to 2,147,483,647) to write.                                   |
|           | float        | Floating point value (from -999999000000 to 999999000000) to write.                           |
|           | String       | String to write (UTF8 string). ThingSpeak limits this field to 255 bytes.                     |
|           | const char * | Character array (zero terminated) to write (UTF8). ThingSpeak limits this field to 255 bytes. |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setStatus
Set the status of a multi-field update. Use status to provide additonal details when writing a channel update. Additionally, status can be used by the ThingTweet App to send a message to Twitter.
```
int setStatus (status)	
```

| Parameter | Type      | Description                                                                   |          
|--------|:-------------|:------------------------------------------------------------------------------|
| status | const char * | String to write (UTF8). ThingSpeak limits this to 255 bytes.                  |
|        | String       | const character array (zero terminated). ThingSpeak limits this to 255 bytes. |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setLatitude
Set the latitude of a multi-field update.
```
int setLatitude	(latitude)	
```

| Parameter | Type  | Description                                                                |          
|-----------|:------|:---------------------------------------------------------------------------|
| latitude  | float | Latitude of the measurement (degrees N, use negative values for degrees S) |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setLongitude
Set the longitude of a multi-field update.
```
int setLongitude (longitude)	
```

| Parameter | Type  | Description                                                                 |          
|-----------|:------|:----------------------------------------------------------------------------|
| longitude | float | Longitude of the measurement (degrees E, use negative values for degrees W) |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setElevation
Set the elevation of a multi-field update.
```
int setElevation (elevation)	
```

| Parameter | Type      | Description                                         |          
|-----------|:------|:--------------------------------------------------------|
| elevation | float | 	Elevation of the measurement (meters above sea level) |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

## setCreatedAt
Set the created-at date of a multi-field update. The timestamp string must be in the ISO 8601 format. Example "2017-01-12 13:22:54"
```
int setCreatedAt (createdAt)
```

| Parameter | Type         | Description                                                                                      |          
|-----------|:-------------|:-------------------------------------------------------------------------------------------------|
| createdAt | String       | Desired timestamp to be included with the channel update as a String.                            |
|           | const char * | Desired timestamp to be included with the channel update as a character array (zero terminated). |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
Timezones can be set using the timezone hour offset parameter. For example, a timestamp for Eastern Standard Time is: "2017-01-12 13:22:54-05". If no timezone hour offset parameter is used, UTC time is assumed.

## setTwitterTweet
Set the Twitter account and message to use for an update to be tweeted.
```
int setTwitterTweet	(twitter, tweet)	
```

| Parameter | Type         | Description                                                                      |          
|-----------|:-------------|:---------------------------------------------------------------------------------|
| twitter   | String       | Twitter account name as a String.                                                |
|           | const char * | Twitter account name as a character array (zero terminated).                     |
| tweet     | String       | Twitter message as a String (UTF-8) limited to 140 character.                    |
|           | const char * | Twitter message as a character array (zero terminated) limited to 140 character. |

### Returns
HTTP status code of 200 if successful. See Return Codes below for other possible return values.

### Remarks
Prior to using this feature, a twitter account must be linked to your ThingSpeak account. To link your twitter account. login to ThingSpeak and go to Apps -> ThingTweet and click Link Twitter Account.

## readStringField
Read the latest string from a channel. Include the readAPIKey to read a private channel.
```
String readStringField (channelNumber, field, readAPIKey)	
```
```
String readStringField (channelNumber, field)	
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| field         | unsigned int  | Field number (1-8) within the channel to read from.                                            |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Value read (UTF8 string), or empty string if there is an error.

## readFloatField
Read the latest float from a channel. Include the readAPIKey to read a private channel.
```
float readFloatField (channelNumber, field, readAPIKey)	
```
```
float readFloatField (channelNumber, field)	
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| field         | unsigned int  | Field number (1-8) within the channel to read from.                                            |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Value read, or 0 if the field is text or there is an error. Use getLastReadStatus() to get more specific information. Note that NAN, INFINITY, and -INFINITY are valid results. 

## readLongField
Read the latest long from a channel. Include the readAPIKey to read a private channel.
```
long readLongField (channelNumber, field, readAPIKey)	
```
```
long readLongField (channelNumber, field)	
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| field         | unsigned int  | Field number (1-8) within the channel to read from.                                            |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Value read, or 0 if the field is text or there is an error. Use getLastReadStatus() to get more specific information. 

## readIntField
Read the latest int from a channel. Include the readAPIKey to read a private channel.
```
int readIntField (channelNumber, field, readAPIKey)		
```
```
int readIntField (channelNumber, field)		
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| field         | unsigned int  | Field number (1-8) within the channel to read from.                                            |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns
Value read, or 0 if the field is text or there is an error. Use getLastReadStatus() to get more specific information. If the value returned is out of range for an int, the result is undefined. 

## readStatus
Read the latest status from a channel. Include the readAPIKey to read a private channel.
```
String readStatus (channelNumber, readAPIKey)	
```
```
String readStatus (channelNumber)
```

| Parameter     | Type          | Description                                                                                    |          
|---------------|:--------------|:-----------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                 |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns

## String readCreatedAt()
Read the created-at timestamp associated with the latest update to a channel. Include the readAPIKey to read a private channel.
```
String readCreatedAt (channelNumber, readAPIKey)
```
```
String readCreatedAt (channelNumber)	
```

| channelNumber | unsigned long | Channel number                                                                                 |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key |

### Returns

## readRaw
Read a raw response from a channel. Include the readAPIKey to read a private channel.
```
String readRaw (channelNumber, URLSuffix, readAPIKey)	
```
```
String readRaw	(channelNumber, URLSuffix)
```

| Parameter     | Type          | Description                                                                                                        |          
|---------------|:--------------|:-------------------------------------------------------------------------------------------------------------------|
| channelNumber | unsigned long | Channel number                                                                                                     |
| URLSuffix     | String        | Raw URL to write to ThingSpeak as a String. See the documentation at https://thingspeak.com/docs/channels#get_feed |
| readAPIKey    | const char *  | Read API key associated with the channel. If you share code with others, do not share this key.                    |     

### Returns

## getLastReadStatus
Get the status of the previous read.
```
int getLastReadStatus ()	
```

### Returns
See Return Codes below for other possible return values.

## Return Codes
| Value | Meaning                                                                                   |
|-------|:----------------------------------------------------------------------------------------|
| 200   | OK / Success                                                                            |
| 404   | Incorrect API key (or invalid ThingSpeak server address)                                |
| -101  | Value is out of range or string is too long (> 255 characters)                          |
| -201  | Invalid field number specified                                                          |
| -210  | setField() was not called before writeFields()                                          |
| -301  | Failed to connect to ThingSpeak                                                         |
| -302  | Unexpected failure during write to ThingSpeak                                           |
| -303  | Unable to parse response                                                                |
| -304  | Timeout waiting for server to respond                                                   |
| -401  | Point was not inserted (most probable cause is the rate limit of once every 15 seconds) |
|    0  | Other error                                                                             |

## Special Characters
Some characters require '%XX' style URL encoding before sending to ThingSpeak.  The writeField() and writeFields() methods will perform the encoding automatically.  The writeRaw() method will not.

| Character  | Encoding |
|------------|:---------|
|     "      | %22      |
|     %      | %25      |
|     &      | %26      |
|     +      | %2B      |
|     ;      | %3B      |

Control characters, ASCII values 0 though 31, are not accepted by ThingSpeak and will be ignored.  Extended ASCII characters with values above 127 will also be ignored. 

# Additional Examples

The library source includes several examples to help you get started. These are accessible in ThingSpeak library section of the Particle Web IDE.

* **CheerLights:** Reads the latest CheerLights color on ThingSpeak, and sets an RGB LED.
* **ReadLastTemperature:** Reads the latest temperature from the public MathWorks weather station in Natick, MA on ThingSpeak.
* **ReadPrivateChannel:** Reads the latest voltage value from a private channel on ThingSpeak.
* **ReadWeatherStation:** Reads the latest weather data from the public MathWorks weather station in Natick, MA on ThingSpeak.
* **WriteMultipleVoltages:** Reads analog voltages from pins 0-7 and writes them to the 8 fields of a channel on ThingSpeak.
* **WriteVoltage:** Reads an analog voltage from pin 0, converts to a voltage, and writes it to a channel on ThingSpeak.