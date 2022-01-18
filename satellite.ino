/*
  See Readme.md file for overview of hardware setup
*/

// Compilation flags
#define Satellite

// Required for data logger and real time clock
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"

// Required for satellite comms
#include "KIM.h"
#include "msg_kineis_std.h"

// Required for temperature sensor
#include <math.h>

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
#define redLedPin 2
#define greenLedPin 3
#define temperaturePin A0

//StackArray <char> stack;

void setup() {

  initialiseHardware();
  initialiseSdCard();
  initialiseSatellite();
  // TODO: Load PrepasRun.txt from file into PROGMEM memory see https://create.arduino.cc/projecthub/john-bradnam/reducing-your-memory-usage-26ca05
  Serial.println(F("Init complete"));
}

void loop() {

  digitalWrite(greenLedPin, HIGH);

  DateTime now = rtc.now();
  
  // Assemble the data to send
  int analogueValue = analogRead(temperaturePin);
  double temperature = getTemperatureFromThermistor(analogueValue);
   
  String dataString = "";
  dataString.reserve(10);
  dataString += "|";
  dataString += temperature;
  dataString += "°C;";

  char buf[60];
  sprintf(buf, "%04d%02d%02dT%02d%02d%02d%s", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), dataString.c_str());

  String filename = "datalog" + String(fileCounter) + ".txt";
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.println(buf);
    dataFile.close();
    // print to the serial port too:
    Serial.println(buf);
  } else {
    Serial.println(F("Error opening SD card"));
  }

  delay(1000);
  digitalWrite(greenLedPin, LOW);

#ifdef Satellite
  // Work out if it is time to transmit or not
  bool transmit = canTransmit();
  //transmit = true; // Debug code
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
    if (dataFile) {
      dataFile.println(dataPacket);
    }
    Serial.print(F("KIM -- Send data ... "));
    if (kim.send_data(dataPacketConverted, sizeof(dataPacketConverted) - 1) == OK_KIM) {
      Serial.println(F("Message sent"));
    } else {
      Serial.println(F("Error"));
    }
    Serial.println(F("KIM -- Turn OFF"));
    digitalWrite(redLedPin, LOW);
    kim.set_sleepMode(true);
  }
#endif

  if (dataFile) {
    dataFile.close();
  }

  delay(delayTime); // Go to sleep
}

bool canTransmit() {
  bool transmit = false;
  SatellitePass satellitePasses[] = {
    SatellitePass (DateTime (2022, 1, 18, 18, 1, 0), DateTime (2022, 1, 18, 18, 8, 0)),
    SatellitePass (DateTime (2022, 1, 18, 18, 11, 0), DateTime (2022, 1, 18, 18, 19, 0)),
    SatellitePass (DateTime (2022, 1, 18, 18, 36, 0), DateTime (2022, 1, 18, 18, 43, 0)),
    SatellitePass (DateTime (2022, 1, 18, 19, 20, 0), DateTime (2022, 1, 18, 19, 24, 0))
  };
  for (int satellite = 0; satellite < sizeof(satellitePasses) / sizeof(SatellitePass); satellite++) {
    if (satellitePasses[satellite].isInRange(rtc.now())) {
      transmit = true;
    }
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

  if (!rtc.initialized() || rtc.lostPower()) {
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

  DateTime now = rtc.now();  
  char buf[60];
  sprintf(buf, "Current time is %02d/%02d/%04d %02d:%02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
  Serial.println(buf);
  
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

  if (kim.check()) {
    Serial.println(F("KIM -- Check success"));
  } else {
    Serial.println(F("KIM -- Check fail. Please check wiring and jumpers. Freezing."));
    while (1);
  }
  Serial.println();

  kim.set_BAND(BAND, sizeof(BAND) - 1);
  kim.set_FRQ(FRQ, sizeof(FRQ) - 1);
  kim.set_PWR(PWR, sizeof(PWR) - 1);
  kim.set_TCXOWU(TCXOWU, sizeof(TCXOWU) - 1);

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

double getTemperatureFromThermistor(int rawADC) {
  double temperature;
  temperature = log(10000.0*((1024.0/rawADC-1))); 
  temperature = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temperature * temperature ))* temperature );
  temperature = temperature - 273.15;            // Convert Kelvin to Celcius
  return temperature;
}
