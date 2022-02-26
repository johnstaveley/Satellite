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
  // Capture reading every X minutes, place in stack and log to SD card
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

  // If there is a message in the stack and a satellite passing overhead, then transmit the next message from the stack
  // Log the transmission to the SD card
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
      // Send thrice to ensure transmission
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
  delay(delayTime); // Go to sleep until next time check
}

// Routine to work out if a satellite is passing overhead
bool canTransmit() {
  bool transmit = false;  
  SatellitePass satellitePasses[] PROGMEM = {
    SatellitePass (DateTime (2022,2,26,12,39,26), DateTime (2022,2,26,12,43,20)),
    SatellitePass (DateTime (2022,2,26,12,42,55), DateTime (2022,2,26,12,44,46)),
    SatellitePass (DateTime (2022,2,26,17,9,1), DateTime (2022,2,26,17,11,2)),
    SatellitePass (DateTime (2022,2,26,17,10,43), DateTime (2022,2,26,17,14,37)),
    SatellitePass (DateTime (2022,2,26,17,12,39), DateTime (2022,2,26,17,16,56)),
    SatellitePass (DateTime (2022,2,26,17,41,48), DateTime (2022,2,26,17,46,20)),
    SatellitePass (DateTime (2022,2,26,18,21,40), DateTime (2022,2,26,18,23,2)),
    SatellitePass (DateTime (2022,2,26,18,41,52), DateTime (2022,2,26,18,45,27)),
    SatellitePass (DateTime (2022,2,26,18,48,41), DateTime (2022,2,26,18,53,55)),
    SatellitePass (DateTime (2022,2,26,18,51,39), DateTime (2022,2,26,18,57,7)),
    SatellitePass (DateTime (2022,2,26,19,11,7), DateTime (2022,2,26,19,15,5)),
    SatellitePass (DateTime (2022,2,26,19,13,2), DateTime (2022,2,26,19,15,58)),
    SatellitePass (DateTime (2022,2,26,19,21,4), DateTime (2022,2,26,19,25,53)),
    SatellitePass (DateTime (2022,2,26,19,56,57), DateTime (2022,2,26,20,1,56)),
    SatellitePass (DateTime (2022,2,26,20,31,41), DateTime (2022,2,26,20,34,31)),
    SatellitePass (DateTime (2022,2,26,20,35,42), DateTime (2022,2,26,20,38,40)),
    SatellitePass (DateTime (2022,2,26,20,49,19), DateTime (2022,2,26,20,54,37)),
    SatellitePass (DateTime (2022,2,26,20,50,19), DateTime (2022,2,26,20,55,41)),
    SatellitePass (DateTime (2022,2,26,21,2,40), DateTime (2022,2,26,21,4,10)),
    SatellitePass (DateTime (2022,2,26,21,37,14), DateTime (2022,2,26,21,42,6)),
    SatellitePass (DateTime (2022,2,26,22,32,6), DateTime (2022,2,26,22,36,42)),
    SatellitePass (DateTime (2022,2,26,22,32,27), DateTime (2022,2,26,22,35,31)),
    SatellitePass (DateTime (2022,2,27,3,28,9), DateTime (2022,2,27,3,31,40)),
    SatellitePass (DateTime (2022,2,27,3,55,6), DateTime (2022,2,27,3,57,41)),
    SatellitePass (DateTime (2022,2,27,5,5,18), DateTime (2022,2,27,5,10,25)),
    SatellitePass (DateTime (2022,2,27,5,27,51), DateTime (2022,2,27,5,31,27)),
    SatellitePass (DateTime (2022,2,27,6,47,13), DateTime (2022,2,27,6,50,30)),
    SatellitePass (DateTime (2022,2,27,6,57,28), DateTime (2022,2,27,7,1,36)),
    SatellitePass (DateTime (2022,2,27,7,7,16), DateTime (2022,2,27,7,11,48)),
    SatellitePass (DateTime (2022,2,27,8,36,43), DateTime (2022,2,27,8,41,50)),
    SatellitePass (DateTime (2022,2,27,8,47,32), DateTime (2022,2,27,8,52,51)),
    SatellitePass (DateTime (2022,2,27,9,0,25), DateTime (2022,2,27,9,4,39)),
    SatellitePass (DateTime (2022,2,27,9,6,44), DateTime (2022,2,27,9,9,43)),
    SatellitePass (DateTime (2022,2,27,9,47,9), DateTime (2022,2,27,9,52,17)),
    SatellitePass (DateTime (2022,2,27,10,17,51), DateTime (2022,2,27,10,20,44)),
    SatellitePass (DateTime (2022,2,27,10,29,32), DateTime (2022,2,27,10,32,33)),
    SatellitePass (DateTime (2022,2,27,10,40,1), DateTime (2022,2,27,10,45,11)),
    SatellitePass (DateTime (2022,2,27,10,45,53), DateTime (2022,2,27,10,51,17)),
    SatellitePass (DateTime (2022,2,27,11,27,39), DateTime (2022,2,27,11,32,11)),
    SatellitePass (DateTime (2022,2,27,12,21,25), DateTime (2022,2,27,12,24,24)),
    SatellitePass (DateTime (2022,2,27,12,27,14), DateTime (2022,2,27,12,31,29)),
    SatellitePass (DateTime (2022,2,27,16,46,43), DateTime (2022,2,27,16,49,45)),
    SatellitePass (DateTime (2022,2,27,17,1,19), DateTime (2022,2,27,17,5,15)),
    SatellitePass (DateTime (2022,2,27,17,11,24), DateTime (2022,2,27,17,14,58)),
    SatellitePass (DateTime (2022,2,27,18,23,31), DateTime (2022,2,27,18,28,41)),
    SatellitePass (DateTime (2022,2,27,18,26,30), DateTime (2022,2,27,18,30,9)),
    SatellitePass (DateTime (2022,2,27,18,39,49), DateTime (2022,2,27,18,45,17)),
    SatellitePass (DateTime (2022,2,27,18,49,53), DateTime (2022,2,27,18,54,57)),
    SatellitePass (DateTime (2022,2,27,18,51,26), DateTime (2022,2,27,18,54,39)),
    SatellitePass (DateTime (2022,2,27,19,1,53), DateTime (2022,2,27,19,4,11)),
    SatellitePass (DateTime (2022,2,27,19,36,43), DateTime (2022,2,27,19,41,21)),
    SatellitePass (DateTime (2022,2,27,20,2,5), DateTime (2022,2,27,20,3,37)),
    SatellitePass (DateTime (2022,2,27,20,4,54), DateTime (2022,2,27,20,8,57)),
    SatellitePass (DateTime (2022,2,27,20,23,1), DateTime (2022,2,27,20,26,40)),
    SatellitePass (DateTime (2022,2,27,20,28,41), DateTime (2022,2,27,20,33,57)),
    SatellitePass (DateTime (2022,2,27,20,30,24), DateTime (2022,2,27,20,33,37)),  
    SatellitePass (DateTime (2022,2,27,20,38,30), DateTime (2022,2,27,20,43,44)),
    SatellitePass (DateTime (2022,2,27,21,16,6), DateTime (2022,2,27,21,21,16)),
    SatellitePass (DateTime (2022,2,27,22,10,30), DateTime (2022,2,27,22,14,33)),
    SatellitePass (DateTime (2022,2,27,22,19,42), DateTime (2022,2,27,22,24,35))
    // TODO: add in more passes
  };
  for (int satellite = 0; satellite < sizeof(satellitePasses) / sizeof(SatellitePass); satellite++) {
    if (satellitePasses[satellite].isInRange(rtc.now())) {
      transmit = true;
      break;
    }
  }
  return transmit;
}

// Function to create the message to send with error correction code
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
  delay(2000); // Allow the RTC crystal time to stabilise

  if (!rtc.initialized() || rtc.lostPower()) {
    Serial.println(F("RTC is NOT initialized, let's set the time!"));
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.start(); 

  DateTime now = rtc.now();
  char buf[60];
  sprintf(buf, "Current time is %02d/%02d/%04d %02d:%02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
  Serial.println(buf);

}

// Setup LEDs and serial output
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

// Initialise the KIM1 satellite communication shield
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

// Convert Analogue signal to temperature
double getTemperatureFromThermistor(int rawADC) {
  double temperature;
  temperature = log(10000.0 * ((1024.0 / rawADC - 1)));
  temperature = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temperature * temperature )) * temperature );
  temperature = temperature - 273.15;            // Convert Kelvin to Celcius
  return temperature;
}
