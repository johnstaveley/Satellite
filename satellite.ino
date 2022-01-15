/*
  See Readme.md file for overview of hardware setup
*/

// Compilation flags
#define Satellite
#define SDCard
//#define BME280

// Required for data logger
#include <SPI.h>  
#include <SD.h>
#include "RTClib.h"           // Real time clock on data logger board

// Required for BME280
#ifdef BME280
#include <Wire.h>  
#include <Adafruit_BME280.h>
#endif

// Required for satellite comms
#include "KIM.h"
#include "msg_kineis_std.h"   

// data logger

RTC_PCF8523 rtc; // Real time clock
unsigned int fileCounter = 1;
#define cardSelect 10   // SD Card

// Satellite comms
#ifdef Satellite
const char BAND[] = "B1";
const char FRQ[] = "300";
const char PWR[] = "750";
const char TCXOWU[] = "5000";
#if defined(__AVR_ATmega4809__)  // Arduino UNO Wifi Rev2
HardwareSerial &kserial = Serial1;
#else // Arduino UNO and Wemos D1
SoftwareSerial kserial(RX_KIM, TX_KIM);
#endif
KIM kim(&kserial);
#endif
#include "satellite_pass.h"

// General
#define delayTime 59000 // 1 minute minus 1 second to display status LED
#ifdef BME280
Adafruit_BME280 bme;    // uses I2C
#endif
#define redLedPin 2
#define greenLedPin 3

//StackArray <char> stack;

void setup() {

  initialiseHardware();
  initialiseSdCard();
  Wire.begin();
  initialiseBme280();
  initialiseSatellite();
  // TODO: Load PrepasRun.txt from file into PROGMEM memory see https://create.arduino.cc/projecthub/john-bradnam/reducing-your-memory-usage-26ca05  
  Serial.println(F("Init complete"));
}

void loop() {

  digitalWrite(greenLedPin, HIGH);
  
  DateTime now = rtc.now();
  // Assemble the data to send
  String dataString = "";
  dataString.reserve(30);
  #ifdef BME280
  dataString += bme.readTemperature();
  dataString += "°C;";
  dataString += bme.readPressure() / 100.0F;
  dataString += "hPa;";
  dataString += bme.readHumidity();
  dataString += "%;";
  #else
  dataString = "Hello world!";
  #endif

  char buf[60];
  sprintf(buf,"%04d%02d%02dT%02d%02d%02d%s", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), dataString.c_str());

  String filename = "datalog" + String(fileCounter) + ".txt";
#ifdef SDCard
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.println(buf);
    dataFile.close();
    // print to the serial port too:
    Serial.println(buf);
  } else {
    Serial.println(F("Error opening SD card"));
  }
#endif

  delay(1000);
  digitalWrite(greenLedPin, LOW);

#ifdef Satellite
  // Work out if it is time to transmit or not
  bool transmit = canTransmit();
  transmit = true; // TODO Remove debug code
  if (transmit) {
    delay(500);
    kim.set_sleepMode(false);
    digitalWrite(redLedPin, HIGH);
    String payload = String(dataString);
    if (payload.length() > 20) {
      payload = payload.substring(0, 19);
    }
    String dataPacket = createSatelliteMessage(now.day(), now.hour(), now.minute(), payload);
    char dataPacketConverted[62];
    dataPacket.toCharArray(dataPacketConverted, dataPacket.length());
    Serial.print(F("KIM -- Send data ... "));
    if(kim.send_data(dataPacketConverted, sizeof(dataPacketConverted)-1) == OK_KIM){
            Serial.println(F("Message sent"));
    } else {
            Serial.println(F("Error"));
    }
    Serial.println(F("KIM -- Turn OFF"));
    digitalWrite(redLedPin, LOW);
    kim.set_sleepMode(true);
  }
#endif  

  delay(delayTime); // Go to sleep  
}

bool canTransmit() {
  bool transmit = false;
  SatellitePass satellitePasses[] = {
    //SatellitePass (DateTime (2022, 1, 15, 4, 42, 0), DateTime (2022, 1, 15, 4, 44, 0)),
    //SatellitePass (DateTime (2022, 1, 15, 7, 27, 0), DateTime (2022, 1, 15, 7, 31, 0)),
    //SatellitePass (DateTime (2022, 1, 15, 8, 50, 0), DateTime (2022, 1, 15, 8, 54, 0)),
    //SatellitePass (DateTime (2022, 1, 15, 9, 07, 0), DateTime (2022, 1, 15, 9, 10, 0)),
    //SatellitePass (DateTime (2022, 1, 15, 10, 00, 0), DateTime (2022, 1, 15, 10, 03, 0)),
    //SatellitePass (DateTime (2022, 1, 15, 10, 53, 0), DateTime (2022, 1, 15, 10, 58, 0)),
    //SatellitePass (DateTime (2022, 1, 15, 11, 12, 0), DateTime (2022, 1, 15, 11, 18, 0)),
    SatellitePass (DateTime (2022, 1, 15, 18, 26, 0), DateTime (2022, 1, 15, 18, 31, 0)),
    SatellitePass (DateTime (2022, 1, 15, 18, 38, 0), DateTime (2022, 1, 15, 18, 41, 0)),
    SatellitePass (DateTime (2022, 1, 15, 18, 59, 0), DateTime (2022, 1, 15, 19, 04, 0)),
    SatellitePass (DateTime (2022, 1, 15, 19, 50, 0), DateTime (2022, 1, 15, 19, 54, 0)),
    SatellitePass (DateTime (2022, 1, 15, 20, 41, 0), DateTime (2022, 1, 15, 20, 47, 0)),
    SatellitePass (DateTime (2022, 1, 15, 21, 04, 0), DateTime (2022, 1, 15, 21, 10, 0)),
    SatellitePass (DateTime (2022, 1, 15, 21, 29, 0), DateTime (2022, 1, 15, 21, 34, 0))
    };
  for (int satellite = 0; satellite < sizeof(satellitePasses) / sizeof(SatellitePass); satellite++){    
    if (satellitePasses[satellite].isInRange(rtc.now())) { transmit = true; }
  }
  return transmit;
}

String createSatelliteMessage(uint8_t day, uint8_t hour, uint8_t min, String userMessage) {
  ArgosMsgTypeDef_t message;
  int counter;
  uint32_t lon  = 450000; // TODO Replace this with real coordinates
  uint32_t lat  = 25000;
  uint32_t alt  = 65;

  uint8_t userdata[20];
  userMessage.getBytes(userdata, userMessage.length());
  
  vMSGKINEIS_STDV1_cleanPayload(&message);
  u16MSGKINEIS_STDV1_setAcqPeriod(&message, USER_MSG, POSITION_STD_ACQ_PERIOD);
  u16MSGKINEIS_STDV1_setDate(&message, day, hour, min, POSITION_STD_DATE);
  u16MSGKINEIS_STDV1_setLocation(&message, lon, lat, alt, POSITION_STD_LOC);
  u16MSGKINEIS_STDV1_setUserData(&message, userdata, 20, POSITION_STD_USER_DATA);
  vMSGKINEIS_STDV1_setCRC16andBCH32(&message, POSITION_STD_BCH32);

  Serial.println(F("Satellite message:"));
  char buf[3];
  String dataPacketString = "";
  for (counter = 0; counter < ARGOS_FRAME_LENGTH; counter++) {
    sprintf(buf, "%02x", message.payload[counter]);
    dataPacketString += buf;
  }
  dataPacketString.toUpperCase();
  Serial.println(dataPacketString);
  return dataPacketString;
}

void initialiseSdCard() {

#ifdef SDCard
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
    Serial.println(F("RTC is NOT initialized, let's set the time!"));
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
#endif
}

void initialiseBme280() {
#ifdef BME280
  Serial.println(F("Init BME280"));
  unsigned sensorStatus;
  sensorStatus = bme.begin(0x76, &Wire); // Need to set specific address for the BME280 used
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
 #endif
}

void initialiseHardware() {  
  pinMode(redLedPin, OUTPUT); 
  digitalWrite(redLedPin, LOW);
  pinMode(greenLedPin, OUTPUT);
  digitalWrite(greenLedPin, LOW);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    delay(10); // wait for serial port to connect. Needed for native USB port only
  }
}

void initialiseSatellite() {
#ifdef Satellite

  Serial.println(F("Init KIM1 shield"));

  if(kim.check()) {
          Serial.println(F("KIM -- Check success"));
  } else {
          Serial.println(F("KIM -- Check fail. Please check wiring and jumpers. Freezing."));
          while(1);
  }
  Serial.println();

  kim.set_BAND(BAND, sizeof(BAND)-1);
  kim.set_FRQ(FRQ, sizeof(FRQ)-1);
  kim.set_PWR(PWR, sizeof(PWR)-1);
  kim.set_TCXOWU(TCXOWU, sizeof(TCXOWU)-1);

  Serial.print(F("KIM -- Get ID : "));
  Serial.println(kim.get_ID());

  Serial.print(F("KIM -- Get SN : "));
  Serial.println(kim.get_SN());

  Serial.print(F("KIM -- Get FW : "));
  Serial.println(kim.get_FW());

  Serial.print(F("KIM -- Get BAND : "));
  Serial.println(kim.get_BAND());

  Serial.print(F("KIM -- Get PWR : "));
  Serial.println(kim.get_PWR());

  Serial.print(F("KIM -- Get FRQ : "));
  Serial.println(kim.get_FRQ());

  Serial.print(F("KIM -- Get TCXOWU : "));
  Serial.println(kim.get_TCXOWU());
  kim.set_sleepMode(true);  
#endif
  
}
