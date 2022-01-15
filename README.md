# INTRODUCTION

This demo project is used to take temperature and pressure data, store it to a SD card and later send it onto a data centre when a satellite is within range. Written in Arduino C++
A read time clock is used on the data logging board to keep time.

Created 03/01/2022 by John Staveley for IoTNorth

# SOFTWARE SETUP

File data/Prepas.txt will need to be regenerated with up to date Lat/Long and dates in order to accurately predict when the satellites will pass overhead.
The Arduino IDE was used to develop this code and upload it to a board. When the code is first deployed, the time on the real time clock will be set, if a battery is present, it will keep time accurately.

# HARDWARE SETUP
In order to follow this demo project, you need the following hardware configured thus:

- USB 3 socket (provides sufficient power for Kineis board)
- 1x Arduino Uno v3 
- 1x Adafruit Assembled Data Logging shield for Arduino attached to SPI bus as follows:
	- MOSI - pin 11
	- MISO - pin 12
	- CLK - pin 13
	- CS - pin 4
- 1x SD card for the above
- 1x CR1220 coin battery for Real Time Clock
- 1x BME280 attached to I2C
    - SCL - pin A5
	- SDA - pin A4
	- +ve and -ve attached to appropriate pins
- 1x Red LED attached to pin 7
- 1x Green LED attached to pin 6
- 1x Kineis shield v2
	- Jumpers 1,2,3 set to Arduino
	- Jumper 7 set to 5V alim

BME280: https://www.arduino.cc/reference/en/libraries/adafruit-bme280-library/
Data Logger: https://thepihut.com/products/adafruit-assembled-data-logging-shield-for-arduino?variant=27739231185

![Hardware configuration for satellite logger](https://raw.githubusercontent.com/johnstaveley/Satellite/main/SatelliteHardware.png "Circuit diagram")
