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
#define delayTime 58000 // 1 minute between messages allowing for processing
#define redLedPin 2
#define greenLedPin 3
#define temperaturePin A0

#include <QueueList.h>
QueueList <String> queue;
int minutesSinceLastReading;
int messageCounter;

void setup() {
  // set the printer of the stack.
  queue.setPrinter (Serial);
  minutesSinceLastReading = 0;
  messageCounter = 1;
  initialiseHardware();
  initialiseSdCard();
  initialiseSatellite();
  // TODO: Load PrepasRun.txt from file into PROGMEM memory see https://create.arduino.cc/projecthub/john-bradnam/reducing-your-memory-usage-26ca05
  Serial.println(F("Init complete"));
}

void loop() {

  DateTime now = rtc.now();
  String filename = "datalog" + String(fileCounter) + ".txt";
  int readingIntervalMinutes = 10;
  if (minutesSinceLastReading == readingIntervalMinutes) {
    digitalWrite(greenLedPin, HIGH);
    minutesSinceLastReading = 0;
    // Assemble the data to send
    int analogValue = analogRead(temperaturePin);
    double temperature = getTemperatureFromThermistor(analogValue);

    String dataString = "";
    dataString.reserve(10);
    dataString += "|";
    dataString += messageCounter;
    dataString += "|";
    dataString += temperature;
    dataString += "C;";
    messageCounter++;

    char logEntry[60];
    sprintf(logEntry, "%02d/%02d/%04d %02d:%02d:%02d%s", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second(), dataString.c_str());

    if (dataString.length() > 20) {
      dataString = dataString.substring(0, 19);
    }
    String dataPacket = createSatelliteMessage(now.day(), now.hour(), now.minute(), dataString);
    queue.push(dataPacket);
    Serial.println("Number of entries in stack: " + String(queue.count()));
    File dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
      dataFile.println(logEntry);
      dataFile.println(dataPacket);
      dataFile.close();
      Serial.println(logEntry);
    } else {
      Serial.println(F("Error opening SD card"));
    }
    digitalWrite(greenLedPin, LOW);
  }

#ifdef Satellite
  if (!queue.isEmpty() && canTransmit()) {
    File dataFile2 = SD.open(filename, FILE_WRITE);
    Serial.println(F("KIM -- Sending data ... "));
    while (!queue.isEmpty() && canTransmit()) {
      delay(1000);
      kim.set_sleepMode(false);
      digitalWrite(redLedPin, HIGH);
      char dataPacketConverted[62];
      String dataPacketToSend = queue.pop();
      dataPacketToSend.toCharArray(dataPacketConverted, dataPacketToSend.length());
      now = rtc.now();
      char logEntry2[200];
      sprintf(logEntry2, "Sending: %02d/%02d/%04d %02d:%02d:%02d %s", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second(), dataPacketToSend.c_str());
      if (dataFile2) {
        dataFile2.println(logEntry2);
      }      
      Serial.println(logEntry2);
      if (kim.send_data(dataPacketConverted, sizeof(dataPacketConverted) - 1) == OK_KIM) {
        Serial.println(F("Message sent"));
      } else {
        Serial.println(F("Error"));
      }
    }
    Serial.println(F("KIM -- Turn OFF"));
    dataFile2.close();
    digitalWrite(redLedPin, LOW);
    kim.set_sleepMode(true);
  }
#endif

  minutesSinceLastReading++;
  delay(delayTime); // Go to sleep
}

bool canTransmit() {
  bool transmit = false;
  SatellitePass satellitePasses[] PROGMEM = {
    SatellitePass (DateTime (2022,2,6,7,53,51), DateTime (2022,2,6,7,59,10)),
    SatellitePass (DateTime (2022,2,6,8,42,38), DateTime (2022,2,6,8,46,11)),
    SatellitePass (DateTime (2022,2,6,9,7,43), DateTime (2022,2,6,9,12,29)),
    SatellitePass (DateTime (2022,2,6,9,34,29), DateTime (2022,2,6,9,39,28)),
    SatellitePass (DateTime (2022,2,6,9,34,59), DateTime (2022,2,6,9,39,35)),
    SatellitePass (DateTime (2022,2,6,9,55,47), DateTime (2022,2,6,10,0,42)),
    SatellitePass (DateTime (2022,2,6,10,21,42), DateTime (2022,2,6,10,26,58)),
    SatellitePass (DateTime (2022,2,6,11,14,44), DateTime (2022,2,6,11,19,31)),
    SatellitePass (DateTime (2022,2,6,11,36,24), DateTime (2022,2,6,11,41,32)),
    SatellitePass (DateTime (2022,2,6,12,2,45), DateTime (2022,2,6,12,6,24)),
    SatellitePass (DateTime (2022,2,6,13,18,55), DateTime (2022,2,6,13,21,3)),
    SatellitePass (DateTime (2022,2,6,16,11,4), DateTime (2022,2,6,16,12,25)),
    SatellitePass (DateTime (2022,2,6,17,16,4), DateTime (2022,2,6,17,20,9)),
    SatellitePass (DateTime (2022,2,6,17,32,4), DateTime (2022,2,6,17,35,4)),
    SatellitePass (DateTime (2022,2,6,17,46,45), DateTime (2022,2,6,17,51,49)),
    SatellitePass (DateTime (2022,2,6,18,6,7), DateTime (2022,2,6,18,11,2)),
    SatellitePass (DateTime (2022,2,6,18,34,17), DateTime (2022,2,6,18,36,40)),
    SatellitePass (DateTime (2022,2,6,18,54,18), DateTime (2022,2,6,18,59,32)),
    SatellitePass (DateTime (2022,2,6,19,5,52), DateTime (2022,2,6,19,9,11)),
    SatellitePass (DateTime (2022,2,6,19,24,23), DateTime (2022,2,6,19,28,43)),
    SatellitePass (DateTime (2022,2,6,19,27,22), DateTime (2022,2,6,19,32,30)),
    SatellitePass (DateTime (2022,2,6,19,45,50), DateTime (2022,2,6,19,50,16)),
    SatellitePass (DateTime (2022,2,6,19,49,27), DateTime (2022,2,6,19,53,47)),
    SatellitePass (DateTime (2022,2,6,20,10,36), DateTime (2022,2,6,20,15,44)),
    SatellitePass (DateTime (2022,2,6,20,37,49), DateTime (2022,2,6,20,40,12)),
    SatellitePass (DateTime (2022,2,6,21,3,12), DateTime (2022,2,6,21,8,28)),
    SatellitePass (DateTime (2022,2,6,21,28,32), DateTime (2022,2,6,21,34,0)),
    SatellitePass (DateTime (2022,2,6,21,51,32), DateTime (2022,2,6,21,56,6)),
    SatellitePass (DateTime (2022,2,6,22,47,35), DateTime (2022,2,6,22,49,34)),
    SatellitePass (DateTime (2022,2,6,23,12,47), DateTime (2022,2,6,23,15,36))
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

  //Serial.println(F("Satellite message:"));
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
  temperature = log(10000.0 * ((1024.0 / rawADC - 1)));
  temperature = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temperature * temperature )) * temperature );
  temperature = temperature - 273.15;            // Convert Kelvin to Celcius
  return temperature;
}
