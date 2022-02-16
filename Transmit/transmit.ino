/*
  See Readme.md file for overview of hardware setup
*/

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
  int readingIntervalMinutes = 20;
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
      for (int transmission = 0; transmission < 3; transmission++) {
        Serial.println(logEntry2);
        if (kim.send_data(dataPacketConverted, sizeof(dataPacketConverted) - 1) == OK_KIM) {
          Serial.println(F("Message sent"));
          delay(15000);
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

  minutesSinceLastReading++; 
  delay(delayTime); // Go to sleep
}

bool canTransmit() {
  bool transmit = false;
  SatellitePass satellitePasses[] PROGMEM = {
    SatellitePass (DateTime (2022,2,16,17,30,40), DateTime (2022,2,16,17,34,22)),
    SatellitePass (DateTime (2022,2,16,17,54,49), DateTime (2022,2,16,17,58,38)),
    SatellitePass (DateTime (2022,2,16,18,2,49), DateTime (2022,2,16,18,6,55)),
    SatellitePass (DateTime (2022,2,16,18,7,39), DateTime (2022,2,16,18,10,31)),
    SatellitePass (DateTime (2022,2,16,19,10,22), DateTime (2022,2,16,19,14,49)),
    SatellitePass (DateTime (2022,2,16,19,18,57), DateTime (2022,2,16,19,21,58)),
    SatellitePass (DateTime (2022,2,16,19,32,36), DateTime (2022,2,16,19,34,53)),
    SatellitePass (DateTime (2022,2,16,19,34,18), DateTime (2022,2,16,19,38,3)),
    SatellitePass (DateTime (2022,2,16,19,42,57), DateTime (2022,2,16,19,44,15)),
    SatellitePass (DateTime (2022,2,16,19,43,13), DateTime (2022,2,16,19,46,52)),
    SatellitePass (DateTime (2022,2,16,20,4,39), DateTime (2022,2,16,20,8,48)),
    SatellitePass (DateTime (2022,2,16,20,57,10), DateTime (2022,2,16,21,1,35)),
    SatellitePass (DateTime (2022,2,16,21,10,17), DateTime (2022,2,16,21,14,51)),
    SatellitePass (DateTime (2022,2,16,21,45,21), DateTime (2022,2,16,21,49,6)),
    SatellitePass (DateTime (2022,2,16,22,53,40), DateTime (2022,2,16,22,56,10)),
    SatellitePass (DateTime (2022,2,17,3,41,1), DateTime (2022,2,17,3,43,42)),
    SatellitePass (DateTime (2022,2,17,4,53,34), DateTime (2022,2,17,4,56,31)),
    SatellitePass (DateTime (2022,2,17,5,18,21), DateTime (2022,2,17,5,22,38)),
    SatellitePass (DateTime (2022,2,17,7,25,40), DateTime (2022,2,17,7,29,37)),
    SatellitePass (DateTime (2022,2,17,7,50,28), DateTime (2022,2,17,7,54,42)),
    SatellitePass (DateTime (2022,2,17,9,6,13), DateTime (2022,2,17,9,10,25)),
    SatellitePass (DateTime (2022,2,17,9,8,22), DateTime (2022,2,17,9,11,41)),
    SatellitePass (DateTime (2022,2,17,9,26,37), DateTime (2022,2,17,9,29,14)),
    SatellitePass (DateTime (2022,2,17,9,30,52), DateTime (2022,2,17,9,34,13)),
    SatellitePass (DateTime (2022,2,17,9,54,55), DateTime (2022,2,17,9,59,15)),
    SatellitePass (DateTime (2022,2,17,10,47,51), DateTime (2022,2,17,10,52,8)),
    SatellitePass (DateTime (2022,2,17,11,6,0), DateTime (2022,2,17,11,10,32)),
    SatellitePass (DateTime (2022,2,17,11,35,41), DateTime (2022,2,17,11,39,3)),
    SatellitePass (DateTime (2022,2,17,12,48,19), DateTime (2022,2,17,12,50,27)),
    SatellitePass (DateTime (2022,2,17,17,19,19), DateTime (2022,2,17,17,22,41)),
    SatellitePass (DateTime (2022,2,17,17,24,26), DateTime (2022,2,17,17,27,18)),
    SatellitePass (DateTime (2022,2,17,17,38,15), DateTime (2022,2,17,17,41,52)),
    SatellitePass (DateTime (2022,2,17,17,52,32), DateTime (2022,2,17,17,55,11)),
    SatellitePass (DateTime (2022,2,17,18,58,25), DateTime (2022,2,17,19,2,57)),
    SatellitePass (DateTime (2022,2,17,18,59,32), DateTime (2022,2,17,19,1,31)),
    SatellitePass (DateTime (2022,2,17,19,2,58), DateTime (2022,2,17,19,7,7)),
    SatellitePass (DateTime (2022,2,17,19,17,21), DateTime (2022,2,17,19,21,30)),
    SatellitePass (DateTime (2022,2,17,19,21,40), DateTime (2022,2,17,19,23,5)),
    SatellitePass (DateTime (2022,2,17,19,27,3), DateTime (2022,2,17,19,29,4)),
    SatellitePass (DateTime (2022,2,17,19,44,25), DateTime (2022,2,17,19,48,13)),
    SatellitePass (DateTime (2022,2,17,20,36,28), DateTime (2022,2,17,20,40,54)),
    SatellitePass (DateTime (2022,2,17,20,58,22), DateTime (2022,2,17,21,2,54)),
    SatellitePass (DateTime (2022,2,17,21,24,4), DateTime (2022,2,17,21,28,13)),
    SatellitePass (DateTime (2022,2,17,22,19,16), DateTime (2022,2,17,22,21,36)),
    SatellitePass (DateTime (2022,2,17,22,40,52), DateTime (2022,2,17,22,44,2)),
    SatellitePass (DateTime (2022,2,18,4,38,28), DateTime (2022,2,18,4,41,15)),
    SatellitePass (DateTime (2022,2,18,4,47,23), DateTime (2022,2,18,4,51,35)),
    SatellitePass (DateTime (2022,2,18,6,14,3), DateTime (2022,2,18,6,15,32)),
    SatellitePass (DateTime (2022,2,18,6,28,31), DateTime (2022,2,18,6,31,23)),
    SatellitePass (DateTime (2022,2,18,7,14,5), DateTime (2022,2,18,7,17,41)),
    SatellitePass (DateTime (2022,2,18,7,25,30), DateTime (2022,2,18,7,29,21)),
    SatellitePass (DateTime (2022,2,18,8,48,31), DateTime (2022,2,18,8,50,45)),
    SatellitePass (DateTime (2022,2,18,8,54,19), DateTime (2022,2,18,8,58,39)),
    SatellitePass (DateTime (2022,2,18,9,5,18), DateTime (2022,2,18,9,9,12)),
    SatellitePass (DateTime (2022,2,18,9,15,29), DateTime (2022,2,18,9,17,6)),
    SatellitePass (DateTime (2022,2,18,9,34,22), DateTime (2022,2,18,9,38,25)),
    SatellitePass (DateTime (2022,2,18,10,27,5), DateTime (2022,2,18,10,31,29)),
    SatellitePass (DateTime (2022,2,18,10,54,0), DateTime (2022,2,18,10,58,33)),
    SatellitePass (DateTime (2022,2,18,11,14,38), DateTime (2022,2,18,11,18,29)),
    SatellitePass (DateTime (2022,2,18,12,8,48), DateTime (2022,2,18,12,10,53)),
    SatellitePass (DateTime (2022,2,18,12,35,53), DateTime (2022,2,18,12,38,39)),
    SatellitePass (DateTime (2022,2,18,17,8,5), DateTime (2022,2,18,17,11,0)),
    SatellitePass (DateTime (2022,2,18,17,14,4), DateTime (2022,2,18,17,16,53)),
    SatellitePass (DateTime (2022,2,18,17,37,34), DateTime (2022,2,18,17,39,48)),
    SatellitePass (DateTime (2022,2,18,18,31,49), DateTime (2022,2,18,18,36,3)),
    SatellitePass (DateTime (2022,2,18,18,46,32), DateTime (2022,2,18,18,51,7)),
    SatellitePass (DateTime (2022,2,18,18,51,50), DateTime (2022,2,18,18,56,11)),
    SatellitePass (DateTime (2022,2,18,19,11,24), DateTime (2022,2,18,19,13,52)),
    SatellitePass (DateTime (2022,2,18,19,24,28), DateTime (2022,2,18,19,27,41)),
    SatellitePass (DateTime (2022,2,18,20,12,17), DateTime (2022,2,18,20,14,58)),
    SatellitePass (DateTime (2022,2,18,20,15,58), DateTime (2022,2,18,20,20,16)),
    SatellitePass (DateTime (2022,2,18,20,31,24), DateTime (2022,2,18,20,32,36)),
    SatellitePass (DateTime (2022,2,18,20,46,34), DateTime (2022,2,18,20,50,57)),
    SatellitePass (DateTime (2022,2,18,21,3,2), DateTime (2022,2,18,21,7,25)),
    SatellitePass (DateTime (2022,2,18,21,57,17), DateTime (2022,2,18,22,0,42)),
    SatellitePass (DateTime (2022,2,18,22,28,18), DateTime (2022,2,18,22,31,55))
  };
  for (int satellite = 0; satellite < sizeof(satellitePasses) / sizeof(SatellitePass); satellite++) {
    if (satellitePasses[satellite].isInRange(rtc.now())) {
      transmit = true;
      break;
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
}

double getTemperatureFromThermistor(int rawADC) {
  double temperature;
  temperature = log(10000.0 * ((1024.0 / rawADC - 1)));
  temperature = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temperature * temperature )) * temperature );
  temperature = temperature - 273.15;            // Convert Kelvin to Celcius
  return temperature;
}
