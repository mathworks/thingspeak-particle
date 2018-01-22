#ThingSpeak Communication Library for Particle

This library enables Particle hardware to write or read data to or from ThingSpeak, an open data platform for the Internet of Things with MATLAB analytics and visualization.

Complete technical documentation is in the doc folder inside the library.

ThingSpeak offers free data storage and analysis of time-stamped numeric or alphanumeric data. Users can access ThingSpeak by visiting http://thingspeak.com and creating a ThingSpeak user account.

ThingSpeak stores data in channels. Channels support an unlimited number of timestamped observations (think of these as rows in a spreadsheet). Each channel has up to 8 fields (think of these as columns in a speadsheet). Check out this [video](http://www.mathworks.com/videos/introduction-to-thingspeak-107749.html) for an overview.

Channels may be public, where anyone can see the data, or private, where only the owner and select users can read the data. Each channel has an associated Write API Key that is used to control who can write to a channel. In addition, private channels have one or more Read API Keys to control who can read from private channel. An API Key is not required to read from public channels.  Each channel can have up to 8 fields. One field is created by default.

You can visualize and do online analytics of your data on ThingSpeak using the built in version of MATLAB, or use the desktop version of MATLAB to get deeper historical insight. Visit https://www.mathworks.com/hardware-support/thingspeak.html to learn more.

#### Particle Web IDE
In the Particle Web IDE, click the libraries tab, find ThingSpeak, and choose "Include in App"

## Compatible Hardware:
* Particle (Formally Spark) Core, [Photon](https://www.particle.io/prototype#photon), [Electron](https://www.particle.io/prototype#electron) and [P1](https://www.particle.io/prototype#p0-and-p1).

## Examples:

The library includes several examples to help you get started. These are accessible in ThingSpeak library section of the Particle Web IDE.

* **CheerLights:** Reads the latest CheerLights color on ThingSpeak, and sets an RGB LED.
* **ReadLastTemperature:** Reads the latest temperature from the public MathWorks weather station in Natick, MA on ThingSpeak.
* **ReadPrivateChannel:** Reads the latest voltage value from a private channel on ThingSpeak.
* **ReadWeatherStation:** Reads the latest weather data from the public MathWorks weather station in Natick, MA on ThingSpeak.
* **WriteMultipleVoltages:** Reads analog voltages from pins 0-7 and writes them to the 8 fields of a channel on ThingSpeak.
* **WriteVoltage:** Reads an analog voltage from pin 0, converts to a voltage, and writes it to a channel on ThingSpeak.
