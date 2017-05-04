# iotsaSmartMeter - web interface to dutch smart meter

iotsaSmartMeter reads electricity and gas usage of a dutch Smart Meter through the standardised P1 port and makes the data available on the net.

Home page is <https://github.com/cwi-dis/iotsaSmartMeter>. 
This software is licensed under the [MIT license](LICENSE.txt) by the   CWI DIS group, <http://www.dis.cwi.nl>.

Details on the standardised P1 port on dutch smart meters (including protocol, pinout, etc) can be found at <http://files.domoticaforum.eu/uploads/Smartmetering/DSMR%20v4.0%20final%20P1.pdf>
## Software requirements

* Arduino IDE, v1.6 or later.
* The iotsa framework, download from <https://github.com/cwi-dis/iotsa>.

## Hardware requirements

* a iotsa board.
* A RJ25 6P6C connector (often incorrectly called RJ11 6P6C).

## Hardware construction

Instructions for constructing the hardware are provided in the _extras_ subfolder. [p1reader-board.pdf](extras/p1reader-board.pdf) has the hardware diagram for the iotsa board. The [Fritzing](http://fritzing.org/home/) project is also available as [p1reader-board.fzz](extras/p1reader-board.fzz).

The P1 port provides power, but unfortunately not enough for the ESP-12 to operate under all circumstances, so you will have to add an external power supply.

## Building the software

Unless you have used different GPIO pins there is nothing to configure.

The _P1Parser_ class parses most common P1 fields. If there are more fields you want: modify `P1Parser::next()`, after consulting the P1 documentation referenced above.

Compile, and flash either using an FTDI or over-the-air.

## Operation

The first time the board boots it creates a Wifi network with a name similar to _config-iotsa1234_.  Connect a device to that network and visit <http://192.168.4.1>. Configure your device name (using the name _smartmeter_ is suggested), WiFi name and password, and after reboot the iotsa board should connect to your network and be visible as <http://smartmeter.local>.

Accessing <http://smartmeter.local/p1> will read a raw P1 telegram in text format.

Accessing <http://smartmeter.local/p1?format=xml> will read a P1 telegram and return the data in XML form.

Accessing <http://smartmeter.local/p1?format=json> will read a P1 telegram and return the data in JSON form.

Each call will result in a new telegram being requested from the smart meter, so there may be a delay of up to 10 seconds before the data is returned (as per the P1 standard).