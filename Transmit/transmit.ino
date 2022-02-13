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
const char PWR[] = "1000";
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
  int readingIntervalMinutes = 15;
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
      memset(dataPacketConverted, 0, sizeof(dataPacketConverted));
      String dataPacketToSend = queue.pop();
      dataPacketToSend.toCharArray(dataPacketConverted, dataPacketToSend.length());
      now = rtc.now();
      char logEntry2[200];
      memset(logEntry2, 0, sizeof(logEntry2));
      sprintf(logEntry2, "Sending: %02d/%02d/%04d %02d:%02d:%02d %s", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second(), dataPacketToSend.c_str());
      if (dataFile2) {
        dataFile2.println(logEntry2);
      }
      // Send twice to ensure transmission
      for (int transmission = 0; transmission < 2; transmission++) {
        Serial.println(logEntry2);
        if (kim.send_data(dataPacketConverted, sizeof(dataPacketConverted) - 1) == OK_KIM) {
          Serial.println(F("Message sent"));
        } else {
          Serial.println(F("Error"));
        }
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
    SatellitePass (DateTime (2022,2,13,9,55,28), DateTime (2022,2,13,9,57,16)),
    SatellitePass (DateTime (2022,2,13,10,14,11), DateTime (2022,2,13,10,17,39)),
    SatellitePass (DateTime (2022,2,13,10,31,7), DateTime (2022,2,13,10,34,49)),
    SatellitePass (DateTime (2022,2,13,11,18,59), DateTime (2022,2,13,11,21,57)),
    SatellitePass (DateTime (2022,2,13,11,55,7), DateTime (2022,2,13,11,58,15)),
    SatellitePass (DateTime (2022,2,13,17,40,8), DateTime (2022,2,13,17,42,54)),
    SatellitePass (DateTime (2022,2,13,17,49,29), DateTime (2022,2,13,17,52,21)),
    SatellitePass (DateTime (2022,2,13,18,5,59), DateTime (2022,2,13,18,9,36)),
    SatellitePass (DateTime (2022,2,13,18,54,12), DateTime (2022,2,13,18,56,24)),
    SatellitePass (DateTime (2022,2,13,19,19,10), DateTime (2022,2,13,19,22,32)),
    SatellitePass (DateTime (2022,2,13,19,28,45), DateTime (2022,2,13,19,31,49)),
    SatellitePass (DateTime (2022,2,13,19,28,48), DateTime (2022,2,13,19,31,8)),
    SatellitePass (DateTime (2022,2,13,19,47,42), DateTime (2022,2,13,19,50,31)),
    SatellitePass (DateTime (2022,2,13,20,7,42), DateTime (2022,2,13,20,10,25)),
    SatellitePass (DateTime (2022,2,13,20,19,57), DateTime (2022,2,13,20,23,34)),
    SatellitePass (DateTime (2022,2,13,21,7,16), DateTime (2022,2,13,21,10,56)),
    SatellitePass (DateTime (2022,2,13,21,47,9), DateTime (2022,2,13,21,50,51)),
    SatellitePass (DateTime (2022,2,13,22,1,52), DateTime (2022,2,13,22,4,1)),
    SatellitePass (DateTime (2022,2,14,3,36,46), DateTime (2022,2,14,3,37,32)),
    SatellitePass (DateTime (2022,2,14,4,7,32), DateTime (2022,2,14,4,8,36)),
    SatellitePass (DateTime (2022,2,14,5,12,47), DateTime (2022,2,14,5,16,23)),
    SatellitePass (DateTime (2022,2,14,5,40,25), DateTime (2022,2,14,5,42,29)),
    SatellitePass (DateTime (2022,2,14,7,27,21), DateTime (2022,2,14,7,30,23)),
    SatellitePass (DateTime (2022,2,14,8,1,30), DateTime (2022,2,14,8,5,15)),
    SatellitePass (DateTime (2022,2,14,9,7,6), DateTime (2022,2,14,9,10,14)),
    SatellitePass (DateTime (2022,2,14,9,18,20), DateTime (2022,2,14,9,21,4)),
    SatellitePass (DateTime (2022,2,14,9,43,9), DateTime (2022,2,14,9,45,35)),
    SatellitePass (DateTime (2022,2,14,10,2,26), DateTime (2022,2,14,10,5,34)),
    SatellitePass (DateTime (2022,2,14,10,10,22), DateTime (2022,2,14,10,14,4)),
    SatellitePass (DateTime (2022,2,14,10,57,58), DateTime (2022,2,14,11,1,22)),
    SatellitePass (DateTime (2022,2,14,11,42,57), DateTime (2022,2,14,11,46,21)),
    SatellitePass (DateTime (2022,2,14,11,52,0), DateTime (2022,2,14,11,53,42)),
    SatellitePass (DateTime (2022,2,14,17,16,17), DateTime (2022,2,14,17,17,53)),
    SatellitePass (DateTime (2022,2,14,17,20,4), DateTime (2022,2,14,17,21,0)),
    SatellitePass (DateTime (2022,2,14,17,54,25), DateTime (2022,2,14,17,57,49)),
    SatellitePass (DateTime (2022,2,14,18,38,47), DateTime (2022,2,14,18,41,10)),
    SatellitePass (DateTime (2022,2,14,18,53,34), DateTime (2022,2,14,18,57,13)),
    SatellitePass (DateTime (2022,2,14,18,57,22), DateTime (2022,2,14,19,0,53)),
    SatellitePass (DateTime (2022,2,14,19,9,44), DateTime (2022,2,14,19,10,39)),
    SatellitePass (DateTime (2022,2,14,19,35,21), DateTime (2022,2,14,19,38,34)),
    SatellitePass (DateTime (2022,2,14,19,56,21), DateTime (2022,2,14,19,58,33)),
    SatellitePass (DateTime (2022,2,14,19,59,39), DateTime (2022,2,14,20,2,55)),
    SatellitePass (DateTime (2022,2,14,20,46,25), DateTime (2022,2,14,20,50,11)),
    SatellitePass (DateTime (2022,2,14,21,35,1), DateTime (2022,2,14,21,38,51)),
    SatellitePass (DateTime (2022,2,14,21,40,3), DateTime (2022,2,14,21,43,7))
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
  memset(userdata, 0, sizeof(userdata));
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
    memset(buf, 0, sizeof(buf));
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
