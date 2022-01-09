# INTRODUCTION

This demo project is used to take temperature and pressure data, store it to a SD card and later send it on when a satellite is within range. Written in Arduino C++

Created 03/01/2022 by John Staveley for IoTNorth

# HARDWARE SETUP
In order to follow this demo project, you need the following hardware configured thus:

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

![Hardware configuration for satellite logger](https://raw.githubusercontent.com/johnstaveley/Satellite/main/SatelliteHardware.png "Circuit diagram")

  