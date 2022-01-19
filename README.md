# INTRODUCTION

This demo project is used to take temperature data, store it to a SD card and later send it onto a data centre when a satellite is within range. Written in Arduino C++ and deployed to an Arduino Uno Wifi v2.
A read time clock is used on the data logging board to keep time.

Created 03/01/2022 by John Staveley for IoTNorth

# SOFTWARE SETUP

File data/Prepas.txt will need to be regenerated with up to date Lat/Long and dates in order to accurately predict when the satellites will pass overhead. Or get this from their website: https://argos-system.cls.fr/argos-cwi2/main.html
The Arduino IDE was used to develop this code and upload it to a board. When the code is first deployed, the time on the real time clock will be set, if a battery is present, it will keep time accurately.

# HARDWARE SETUP
In order to follow this demo project, you need the following hardware configured thus:

- USB 2 socket (provides sufficient power for Kineis board). Not USB1. Or battery capable of delivering 1Amp
- 1x Arduino Uno WiFi v2
- 1x Adafruit Assembled Data Logging shield for Arduino attached to SPI bus as follows:
	- MOSI - pin 11
	- MISO - pin 12
	- CLK - pin 13
	- CS - pin 4
- 1x SD card for the above
- 1x CR1220 coin battery for Real Time Clock on data logging shield
- 1x HW-498 Temperature sensor
    - +ve pin to %v
	- -ve pin to ground
	- Signal pin to A0
- 1x HW-477 LED
	- 1x Red LED attached to pin 7
	- 1x Green LED attached to pin 6
	- -ve connected to ground
- 1x Kineis shield v2
	- Jumpers 1,7 set to Arduino
	- Jumpers 2, 3 set to STM32

Data Logger: https://thepihut.com/products/adafruit-assembled-data-logging-shield-for-arduino?variant=27739231185

![Hardware configuration for satellite logger](https://raw.githubusercontent.com/johnstaveley/Satellite/main/SatelliteHardware.png "Circuit diagram")

# KNOWN ISSUES

- Sometimes the RTC loses power and resets to the script time. This will cause the data to not be sent at the correct time. A redeploy will (usually) solve this.

