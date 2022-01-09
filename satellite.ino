/*
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  BME280 attached to I2C
  Red LED attached to pin 7, Green LED attached to pin 6

  created  03/01/2022
  by John Staveley

*/

#include <Wire.h>
#include <SPI.h> // Required for data logger
#include <SD.h>  // Required for data logger
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "RTClib.h" // Real time clock on data logger board

RTC_PCF8523 rtc; // Real time clock
#define SEALEVELPRESSURE_HPA (1013.25) // BME280hi
Adafruit_BME280 bme;     // uses I2C
#define cardSelect 10 // SD Card
#define delayTime 59000 // 1 minute minus 1 second to display status LED
#define redLedPin 7
#define greenLedPin 6
unsigned int fileCounter = 1;

void setup() {

  pinMode(redLedPin, OUTPUT); 
  digitalWrite(redLedPin, LOW);
  pinMode(greenLedPin, OUTPUT); 
  digitalWrite(greenLedPin, LOW);
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println(F("Init SD Card"));

  // see if the card is present and can be initialized:
  if (!SD.begin(cardSelect)) {
    Serial.println(F("SD Card failed, or not present"));
    // don't do anything more:
    while (1) delay(10);
  }

  Serial.println(F("Init RTC"));
  if (! rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    //
    // Note: allow 2 seconds after inserting battery or applying external power
    // without battery before calling adjust(). This gives the PCF8523's
    // crystal oscillator time to stabilize. If you call adjust() very quickly
    // after the RTC is powered, lostPower() may still return true.
  }

  rtc.start();

  Wire.begin();

  Serial.println(F("Init BME280"));
  unsigned sensorStatus;
  sensorStatus = bme.begin(0x76, &Wire); // Need to set specific address for the BME280 I used
  if (!sensorStatus) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"));
    Serial.print(F("SensorID was: 0x"));
    Serial.println(bme.sensorID(), 16);
    //Serial.print(F(" ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n"));
    //Serial.println(F(" ID 0x56-0x58 = BMP280"));
    //Serial.println(F(" ID 0x60 = BME280"));
    //Serial.println(F(" ID 0x61 = BME680"));
    while (1) delay(10);
  }  

  // TODO: Load PrepasRun.txt from file into PROGMEM memory see https://create.arduino.cc/projecthub/john-bradnam/reducing-your-memory-usage-26ca05  

  Serial.println(F("Init complete"));
}

void loop() {

  digitalWrite(greenLedPin, HIGH);
  
  DateTime now = rtc.now();
  // make a string for assembling the data to log:
  String dataString = "";
  dataString.reserve(30);
  dataString += bme.readTemperature();
  dataString += "°C, ";
  dataString += bme.readPressure() / 100.0F;
  dataString += "hPa, ";
  dataString += bme.readAltitude(SEALEVELPRESSURE_HPA);
  dataString += "m, ";
  dataString += bme.readHumidity();
  dataString += "%";

  char buf[60];
  sprintf(buf,"%02d/%02d/%04d %02d:%02d:%02d %s;", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second(), dataString.c_str());

  // open the file. note that only one file can be open at a time
  // so you have to close this one before opening another.
  String filename = "datalog" + String(fileCounter) + ".txt";
  File dataFile = SD.open(filename, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(buf);
    dataFile.close();
    // print to the serial port too:
    Serial.println(buf);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println(F("Error opening datalog.txt"));
  }  

  delay(1000);
  digitalWrite(greenLedPin, LOW);

  // TODO: Work out if it is time to transmit or not
  bool transmit = false;
  if (transmit) {
    //  If transmitting, light the RED LED
    digitalWrite(redLedPin, HIGH);
    delay(500); // TODO: Remove this artificial delay
    // Load data to send
    File dataFile = SD.open(filename);
    if (dataFile) {
        String dataToWrite;
        while (dataFile.available()) {
          dataToWrite += dataFile.read();
        }
        dataFile.close();
        
        // TODO: Separate payloads to send delimited by ;
        // TODO: For each payload, Create Kineis data structure and send to satellite
    }
    
    // Set new filename as old has been processed
    fileCounter++;
    digitalWrite(redLedPin, LOW);
  }
  delay(delayTime); // Go to sleep  
}
