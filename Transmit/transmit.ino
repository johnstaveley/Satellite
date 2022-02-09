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
    SatellitePass (DateTime (2022,2,9,4,29,30), DateTime (2022,2,9,4,32,43)),
    SatellitePass (DateTime (2022,2,9,5,22,9), DateTime (2022,2,9,5,24,32)),
    SatellitePass (DateTime (2022,2,9,6,9,39), DateTime (2022,2,9,6,12,17)),
    SatellitePass (DateTime (2022,2,9,7,20,41), DateTime (2022,2,9,7,23,43)),
    SatellitePass (DateTime (2022,2,9,7,53,34), DateTime (2022,2,9,7,57,8)),
    SatellitePass (DateTime (2022,2,9,9,1,3), DateTime (2022,2,9,9,4,30)),
    SatellitePass (DateTime (2022,2,9,9,21,50), DateTime (2022,2,9,9,24,46)),
    SatellitePass (DateTime (2022,2,9,9,34,28), DateTime (2022,2,9,9,36,34)),
    SatellitePass (DateTime (2022,2,9,10,14,4), DateTime (2022,2,9,10,17,46)),
    SatellitePass (DateTime (2022,2,9,11,1,44), DateTime (2022,2,9,11,5,0)),
    SatellitePass (DateTime (2022,2,9,11,2,2), DateTime (2022,2,9,11,5,53)),
    SatellitePass (DateTime (2022,2,9,11,56,23), DateTime (2022,2,9,11,57,18)),
    SatellitePass (DateTime (2022,2,9,17,14,31), DateTime (2022,2,9,17,16,25)),
    SatellitePass (DateTime (2022,2,9,17,41,4), DateTime (2022,2,9,17,43,50)),
    SatellitePass (DateTime (2022,2,9,18,14,6), DateTime (2022,2,9,18,17,31)),
    SatellitePass (DateTime (2022,2,9,18,21,16), DateTime (2022,2,9,18,23,41)),
    SatellitePass (DateTime (2022,2,9,18,52,46), DateTime (2022,2,9,18,56,38)),
    SatellitePass (DateTime (2022,2,9,19,12,49), DateTime (2022,2,9,19,13,52)),
    SatellitePass (DateTime (2022,2,9,19,20,7), DateTime (2022,2,9,19,23,29)),
    SatellitePass (DateTime (2022,2,9,19,54,26), DateTime (2022,2,9,19,56,37)),
    SatellitePass (DateTime (2022,2,9,20,2,52), DateTime (2022,2,9,20,6,11)),
    SatellitePass (DateTime (2022,2,9,20,49,40), DateTime (2022,2,9,20,53,24)),
    SatellitePass (DateTime (2022,2,9,20,54,5), DateTime (2022,2,9,20,57,50)),
    SatellitePass (DateTime (2022,2,9,21,43,29), DateTime (2022,2,9,21,46,24)),
    SatellitePass (DateTime (2022,2,9,22,36,46), DateTime (2022,2,9,22,38,54)),
    SatellitePass (DateTime (2022,2,10,3,59,34), DateTime (2022,2,10,4,1,54)),
    SatellitePass (DateTime (2022,2,10,5,6,48), DateTime (2022,2,10,5,9,15)),
    SatellitePass (DateTime (2022,2,10,5,37,38), DateTime (2022,2,10,5,41,3)),
    SatellitePass (DateTime (2022,2,10,7,9,15), DateTime (2022,2,10,7,11,47)),
    SatellitePass (DateTime (2022,2,10,7,28,39), DateTime (2022,2,10,7,31,46)),
    SatellitePass (DateTime (2022,2,10,8,49,6), DateTime (2022,2,10,8,52,45)),
    SatellitePass (DateTime (2022,2,10,9,1,56), DateTime (2022,2,10,9,3,52)),
    SatellitePass (DateTime (2022,2,10,9,8,34), DateTime (2022,2,10,9,11,33)),
    SatellitePass (DateTime (2022,2,10,9,53,25), DateTime (2022,2,10,9,57,1)),
    SatellitePass (DateTime (2022,2,10,10,40,47), DateTime (2022,2,10,10,44,23)),
    SatellitePass (DateTime (2022,2,10,10,50,3), DateTime (2022,2,10,10,53,54)),
    SatellitePass (DateTime (2022,2,10,11,34,29), DateTime (2022,2,10,11,36,49)),
    SatellitePass (DateTime (2022,2,10,12,32,47), DateTime (2022,2,10,12,34,0)),
    SatellitePass (DateTime (2022,2,10,17,3,53), DateTime (2022,2,10,17,4,45)),
    SatellitePass (DateTime (2022,2,10,17,17,18), DateTime (2022,2,10,17,18,51)),
    SatellitePass (DateTime (2022,2,10,17,43,34), DateTime (2022,2,10,17,46,18)),
    SatellitePass (DateTime (2022,2,10,18,6,6), DateTime (2022,2,10,18,8,24)),
    SatellitePass (DateTime (2022,2,10,18,40,54), DateTime (2022,2,10,18,44,48)),
    SatellitePass (DateTime (2022,2,10,18,54,31), DateTime (2022,2,10,18,58,10)),
    SatellitePass (DateTime (2022,2,10,19,22,40), DateTime (2022,2,10,19,25,49)),
    SatellitePass (DateTime (2022,2,10,19,42,48), DateTime (2022,2,10,19,45,37)),
    SatellitePass (DateTime (2022,2,10,20,29,1), DateTime (2022,2,10,20,32,43)),
    SatellitePass (DateTime (2022,2,10,20,42,18), DateTime (2022,2,10,20,45,54)),
    SatellitePass (DateTime (2022,2,10,21,22,7), DateTime (2022,2,10,21,25,34)),
    SatellitePass (DateTime (2022,2,10,22,12,9), DateTime (2022,2,10,22,13,19)),
    SatellitePass (DateTime (2022,2,10,22,24,1), DateTime (2022,2,10,22,26,48)),
    SatellitePass (DateTime (2022,2,11,4,51,37), DateTime (2022,2,11,4,54,0)),
    SatellitePass (DateTime (2022,2,11,5,6,21), DateTime (2022,2,11,5,9,57)),
    SatellitePass (DateTime (2022,2,11,6,58,5), DateTime (2022,2,11,6,59,50)),
    SatellitePass (DateTime (2022,2,11,7,4,12), DateTime (2022,2,11,7,6,20)),
    SatellitePass (DateTime (2022,2,11,8,37,11), DateTime (2022,2,11,8,40,57)),
    SatellitePass (DateTime (2022,2,11,8,42,59), DateTime (2022,2,11,8,46,27)),
    SatellitePass (DateTime (2022,2,11,9,32,59), DateTime (2022,2,11,9,36,12)),
    SatellitePass (DateTime (2022,2,11,10,20,0), DateTime (2022,2,11,10,23,42)),
    SatellitePass (DateTime (2022,2,11,10,38,6), DateTime (2022,2,11,10,41,54)),
    SatellitePass (DateTime (2022,2,11,11,13,14), DateTime (2022,2,11,11,16,16)),
    SatellitePass (DateTime (2022,2,11,12,20,4), DateTime (2022,2,11,12,22,9)),
    SatellitePass (DateTime (2022,2,11,17,51,4), DateTime (2022,2,11,17,53,5)),
    SatellitePass (DateTime (2022,2,11,18,29,7), DateTime (2022,2,11,18,32,58)),
    SatellitePass (DateTime (2022,2,11,18,29,17), DateTime (2022,2,11,18,32,56)),
    SatellitePass (DateTime (2022,2,11,18,51,21), DateTime (2022,2,11,18,54,54)),
    SatellitePass (DateTime (2022,2,11,19,23,12), DateTime (2022,2,11,19,25,5)),
    SatellitePass (DateTime (2022,2,11,19,25,56), DateTime (2022,2,11,19,26,58)),
    SatellitePass (DateTime (2022,2,11,20,8,37), DateTime (2022,2,11,20,12,5)),
    SatellitePass (DateTime (2022,2,11,20,12,41), DateTime (2022,2,11,20,13,15)),
    SatellitePass (DateTime (2022,2,11,20,13,21), DateTime (2022,2,11,20,14,13)),
    SatellitePass (DateTime (2022,2,11,20,30,37), DateTime (2022,2,11,20,33,58)),
    SatellitePass (DateTime (2022,2,11,21,1,4), DateTime (2022,2,11,21,4,46)),
    SatellitePass (DateTime (2022,2,11,21,49,45), DateTime (2022,2,11,21,52,24)),
    SatellitePass (DateTime (2022,2,11,22,11,31), DateTime (2022,2,11,22,14,43))
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
