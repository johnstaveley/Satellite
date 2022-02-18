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
    SatellitePass (DateTime (2022,2,18,16,53,31), DateTime (2022,2,18,16,55,55)),
SatellitePass (DateTime (2022,2,18,17,6,53), DateTime (2022,2,18,17,10,59)),
SatellitePass (DateTime (2022,2,18,17,12,55), DateTime (2022,2,18,17,16,53)),
SatellitePass (DateTime (2022,2,18,17,36,43), DateTime (2022,2,18,17,39,48)),
SatellitePass (DateTime (2022,2,18,18,31,7), DateTime (2022,2,18,18,36,11)),
SatellitePass (DateTime (2022,2,18,18,38,33), DateTime (2022,2,18,18,41,4)),
SatellitePass (DateTime (2022,2,18,18,45,36), DateTime (2022,2,18,18,51,4)),
SatellitePass (DateTime (2022,2,18,18,50,56), DateTime (2022,2,18,18,56,10)),
SatellitePass (DateTime (2022,2,18,19,8,39), DateTime (2022,2,18,19,11,17)),
SatellitePass (DateTime (2022,2,18,19,10,37), DateTime (2022,2,18,19,13,54)),
SatellitePass (DateTime (2022,2,18,19,23,26), DateTime (2022,2,18,19,27,44)),
SatellitePass (DateTime (2022,2,18,20,11,14), DateTime (2022,2,18,20,15,7)),
SatellitePass (DateTime (2022,2,18,20,15,0), DateTime (2022,2,18,20,20,10)),
SatellitePass (DateTime (2022,2,18,20,29,10), DateTime (2022,2,18,20,32,32)),
SatellitePass (DateTime (2022,2,18,20,34,1), DateTime (2022,2,18,20,36,47)),
SatellitePass (DateTime (2022,2,18,20,45,39), DateTime (2022,2,18,20,50,56)),
SatellitePass (DateTime (2022,2,18,21,2,10), DateTime (2022,2,18,21,7,27)),
SatellitePass (DateTime (2022,2,18,21,56,3), DateTime (2022,2,18,22,0,34)),
SatellitePass (DateTime (2022,2,18,22,27,9), DateTime (2022,2,18,22,31,53)),
SatellitePass (DateTime (2022,2,18,22,46,25), DateTime (2022,2,18,22,48,31)),
SatellitePass (DateTime (2022,2,19,4,16,7), DateTime (2022,2,19,4,20,48)),
SatellitePass (DateTime (2022,2,19,4,22,46), DateTime (2022,2,19,4,26,3)),
SatellitePass (DateTime (2022,2,19,5,55,22), DateTime (2022,2,19,6,0,12)),
SatellitePass (DateTime (2022,2,19,5,56,58), DateTime (2022,2,19,6,0,7)),
SatellitePass (DateTime (2022,2,19,6,59,42), DateTime (2022,2,19,7,3,51)),
SatellitePass (DateTime (2022,2,19,7,1,20), DateTime (2022,2,19,7,5,40)),
SatellitePass (DateTime (2022,2,19,8,27,17), DateTime (2022,2,19,8,29,38)),
SatellitePass (DateTime (2022,2,19,8,38,57), DateTime (2022,2,19,8,44,4)),
SatellitePass (DateTime (2022,2,19,8,41,27), DateTime (2022,2,19,8,46,49)),
SatellitePass (DateTime (2022,2,19,9,2,19), DateTime (2022,2,19,9,4,52)),
SatellitePass (DateTime (2022,2,19,9,12,58), DateTime (2022,2,19,9,17,33)),
SatellitePass (DateTime (2022,2,19,10,5,27), DateTime (2022,2,19,10,10,40)),
SatellitePass (DateTime (2022,2,19,10,20,8), DateTime (2022,2,19,10,22,58)),
SatellitePass (DateTime (2022,2,19,10,23,19), DateTime (2022,2,19,10,26,37)),
SatellitePass (DateTime (2022,2,19,10,41,7), DateTime (2022,2,19,10,46,32)),
SatellitePass (DateTime (2022,2,19,10,52,50), DateTime (2022,2,19,10,57,55)),
SatellitePass (DateTime (2022,2,19,11,46,10), DateTime (2022,2,19,11,50,22)),
SatellitePass (DateTime (2022,2,19,12,22,25), DateTime (2022,2,19,12,26,48)),
SatellitePass (DateTime (2022,2,19,12,34,37), DateTime (2022,2,19,12,36,58)),
SatellitePass (DateTime (2022,2,19,16,48,56), DateTime (2022,2,19,16,51,59)),
SatellitePass (DateTime (2022,2,19,16,55,34), DateTime (2022,2,19,16,59,17)),
SatellitePass (DateTime (2022,2,19,17,21,47), DateTime (2022,2,19,17,24,24)),
SatellitePass (DateTime (2022,2,19,18,0,13), DateTime (2022,2,19,18,5,2)),
SatellitePass (DateTime (2022,2,19,18,20,1), DateTime (2022,2,19,18,20,44)),
SatellitePass (DateTime (2022,2,19,18,25,46), DateTime (2022,2,19,18,30,57)),
SatellitePass (DateTime (2022,2,19,18,33,46), DateTime (2022,2,19,18,39,13)),
SatellitePass (DateTime (2022,2,19,18,55,10), DateTime (2022,2,19,18,58,39)),
SatellitePass (DateTime (2022,2,19,18,57,37), DateTime (2022,2,19,18,59,31)),
SatellitePass (DateTime (2022,2,19,19,3,33), DateTime (2022,2,19,19,7,15)),
SatellitePass (DateTime (2022,2,19,19,39,48), DateTime (2022,2,19,19,44,22)),
SatellitePass (DateTime (2022,2,19,19,54,38), DateTime (2022,2,19,19,59,34)),
SatellitePass (DateTime (2022,2,19,20,7,13), DateTime (2022,2,19,20,11,14)),
SatellitePass (DateTime (2022,2,19,20,16,34), DateTime (2022,2,19,20,20,31)),
SatellitePass (DateTime (2022,2,19,20,33,50), DateTime (2022,2,19,20,38,59)),
SatellitePass (DateTime (2022,2,19,20,41,22), DateTime (2022,2,19,20,46,41)),
SatellitePass (DateTime (2022,2,19,21,34,44), DateTime (2022,2,19,21,39,42)),
SatellitePass (DateTime (2022,2,19,22,14,46), DateTime (2022,2,19,22,19,47)),
SatellitePass (DateTime (2022,2,19,22,23,56), DateTime (2022,2,19,22,27,27)),
SatellitePass (DateTime (2022,2,20,3,46,1), DateTime (2022,2,20,3,50,3)),
SatellitePass (DateTime (2022,2,20,4,7,54), DateTime (2022,2,20,4,10,51)),
SatellitePass (DateTime (2022,2,20,5,23,54), DateTime (2022,2,20,5,29,1)),
SatellitePass (DateTime (2022,2,20,5,41,17), DateTime (2022,2,20,5,44,44)),
SatellitePass (DateTime (2022,2,20,6,35,15), DateTime (2022,2,20,6,38,18)),
SatellitePass (DateTime (2022,2,20,6,49,47), DateTime (2022,2,20,6,53,41)),
SatellitePass (DateTime (2022,2,20,7,7,18), DateTime (2022,2,20,7,9,21)),
SatellitePass (DateTime (2022,2,20,8,13,42), DateTime (2022,2,20,8,18,52)),
SatellitePass (DateTime (2022,2,20,8,29,35), DateTime (2022,2,20,8,35,0)),
SatellitePass (DateTime (2022,2,20,8,51,38), DateTime (2022,2,20,8,52,40)),
SatellitePass (DateTime (2022,2,20,8,52,40), DateTime (2022,2,20,8,56,34)),
SatellitePass (DateTime (2022,2,20,9,44,47), DateTime (2022,2,20,9,49,54)),
SatellitePass (DateTime (2022,2,20,9,54,19), DateTime (2022,2,20,9,58,6)),
SatellitePass (DateTime (2022,2,20,10,11,12), DateTime (2022,2,20,10,14,57)),
SatellitePass (DateTime (2022,2,20,10,29,10), DateTime (2022,2,20,10,34,30)),
SatellitePass (DateTime (2022,2,20,10,32,1), DateTime (2022,2,20,10,37,15)),
SatellitePass (DateTime (2022,2,20,11,25,13), DateTime (2022,2,20,11,29,51)),
SatellitePass (DateTime (2022,2,20,12,10,17), DateTime (2022,2,20,12,14,55)),
SatellitePass (DateTime (2022,2,20,12,13,15), DateTime (2022,2,20,12,16,34)),
SatellitePass (DateTime (2022,2,20,16,25,35), DateTime (2022,2,20,16,27,11)),
SatellitePass (DateTime (2022,2,20,16,44,20), DateTime (2022,2,20,16,47,36)),
SatellitePass (DateTime (2022,2,20,17,7,8), DateTime (2022,2,20,17,8,56)),
SatellitePass (DateTime (2022,2,20,17,29,34), DateTime (2022,2,20,17,33,45)),
SatellitePass (DateTime (2022,2,20,18,0,51), DateTime (2022,2,20,18,5,49)),
SatellitePass (DateTime (2022,2,20,18,22,2), DateTime (2022,2,20,18,27,24)),
SatellitePass (DateTime (2022,2,20,18,39,48), DateTime (2022,2,20,18,43,24)),
SatellitePass (DateTime (2022,2,20,18,43,56), DateTime (2022,2,20,18,46,48)),
SatellitePass (DateTime (2022,2,20,18,47,19), DateTime (2022,2,20,18,47,45)),
SatellitePass (DateTime (2022,2,20,19,8,32), DateTime (2022,2,20,19,13,30)),
SatellitePass (DateTime (2022,2,20,19,34,29), DateTime (2022,2,20,19,39,1)),
SatellitePass (DateTime (2022,2,20,19,41,2), DateTime (2022,2,20,19,45,45)),
SatellitePass (DateTime (2022,2,20,20,4,9), DateTime (2022,2,20,20,8,32)),
SatellitePass (DateTime (2022,2,20,20,20,46), DateTime (2022,2,20,20,25,59)),
SatellitePass (DateTime (2022,2,20,20,22,7), DateTime (2022,2,20,20,27,5)),
SatellitePass (DateTime (2022,2,20,20,49,30), DateTime (2022,2,20,20,51,58)),
SatellitePass (DateTime (2022,2,20,21,13,39), DateTime (2022,2,20,21,18,52)),
SatellitePass (DateTime (2022,2,20,22,2,9), DateTime (2022,2,20,22,6,29)),
SatellitePass (DateTime (2022,2,20,22,2,31), DateTime (2022,2,20,22,7,44))
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
