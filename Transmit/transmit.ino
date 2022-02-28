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
    SatellitePass (DateTime (2022,2,28,2,59,1), DateTime (2022,2,28,3,1,3)),
SatellitePass (DateTime (2022,2,28,3,40,33), DateTime (2022,2,28,3,42,31)),
SatellitePass (DateTime (2022,2,28,4,34,29), DateTime (2022,2,28,4,39,24)),
SatellitePass (DateTime (2022,2,28,5,12,25), DateTime (2022,2,28,5,16,5)),
SatellitePass (DateTime (2022,2,28,6,14,38), DateTime (2022,2,28,6,19,4)),
SatellitePass (DateTime (2022,2,28,6,33,1), DateTime (2022,2,28,6,36,0)),
SatellitePass (DateTime (2022,2,28,6,55,40), DateTime (2022,2,28,6,59,48)),
SatellitePass (DateTime (2022,2,28,8,11,24), DateTime (2022,2,28,8,16,34)),
SatellitePass (DateTime (2022,2,28,8,35,39), DateTime (2022,2,28,8,41,1)),
SatellitePass (DateTime (2022,2,28,8,40,18), DateTime (2022,2,28,8,43,40)),
SatellitePass (DateTime (2022,2,28,8,55,37), DateTime (2022,2,28,8,57,31)),
SatellitePass (DateTime (2022,2,28,9,26,34), DateTime (2022,2,28,9,31,26)),
SatellitePass (DateTime (2022,2,28,9,52,0), DateTime (2022,2,28,9,55,49)),
SatellitePass (DateTime (2022,2,28,10,17,23), DateTime (2022,2,28,10,20,53)),
SatellitePass (DateTime (2022,2,28,10,19,13), DateTime (2022,2,28,10,24,29)),
SatellitePass (DateTime (2022,2,28,10,33,56), DateTime (2022,2,28,10,39,17)),
SatellitePass (DateTime (2022,2,28,11,6,41), DateTime (2022,2,28,11,11,36)),
SatellitePass (DateTime (2022,2,28,12,0,12), DateTime (2022,2,28,12,3,58)),
SatellitePass (DateTime (2022,2,28,12,15,8), DateTime (2022,2,28,12,19,37)),
SatellitePass (DateTime (2022,2,28,12,49,19), DateTime (2022,2,28,12,50,28)),
SatellitePass (DateTime (2022,2,28,16,23,26), DateTime (2022,2,28,16,24,54)),
SatellitePass (DateTime (2022,2,28,16,42,20), DateTime (2022,2,28,16,43,25)),
SatellitePass (DateTime (2022,2,28,16,50,3), DateTime (2022,2,28,16,53,34)),
SatellitePass (DateTime (2022,2,28,17,58,36), DateTime (2022,2,28,18,3,31)),
SatellitePass (DateTime (2022,2,28,18,11,13), DateTime (2022,2,28,18,14,48)),
SatellitePass (DateTime (2022,2,28,18,18,48), DateTime (2022,2,28,18,23,49)),
SatellitePass (DateTime (2022,2,28,18,28,1), DateTime (2022,2,28,18,33,26)),
SatellitePass (DateTime (2022,2,28,18,32,4), DateTime (2022,2,28,18,34,16)),
SatellitePass (DateTime (2022,2,28,18,51,5), DateTime (2022,2,28,18,52,26)),
SatellitePass (DateTime (2022,2,28,19,16,41), DateTime (2022,2,28,19,20,49)),
SatellitePass (DateTime (2022,2,28,19,38,43), DateTime (2022,2,28,19,43,27)),
SatellitePass (DateTime (2022,2,28,19,46,9), DateTime (2022,2,28,19,48,29)),
SatellitePass (DateTime (2022,2,28,19,58,43), DateTime (2022,2,28,20,2,55)),
SatellitePass (DateTime (2022,2,28,20,8,10), DateTime (2022,2,28,20,13,17)),
SatellitePass (DateTime (2022,2,28,20,10,27), DateTime (2022,2,28,20,14,39)),
SatellitePass (DateTime (2022,2,28,20,26,46), DateTime (2022,2,28,20,31,49)),
SatellitePass (DateTime (2022,2,28,20,55,9), DateTime (2022,2,28,21,0,28)),
SatellitePass (DateTime (2022,2,28,21,48,57), DateTime (2022,2,28,21,53,37)),
SatellitePass (DateTime (2022,2,28,22,7,25), DateTime (2022,2,28,22,12,32)),
SatellitePass (DateTime (2022,2,28,22,38,53), DateTime (2022,2,28,22,41,28)),
SatellitePass (DateTime (2022,3,1,3,26,38), DateTime (2022,3,1,3,27,23)),
SatellitePass (DateTime (2022,3,1,4,4,4), DateTime (2022,3,1,4,8,33)),
SatellitePass (DateTime (2022,3,1,4,57,6), DateTime (2022,3,1,5,0,47)),
SatellitePass (DateTime (2022,3,1,5,42,50), DateTime (2022,3,1,5,47,48)),
SatellitePass (DateTime (2022,3,1,6,33,45), DateTime (2022,3,1,6,35,16)),
SatellitePass (DateTime (2022,3,1,6,44,10), DateTime (2022,3,1,6,47,49)),
SatellitePass (DateTime (2022,3,1,7,46,13), DateTime (2022,3,1,7,51,17)),
SatellitePass (DateTime (2022,3,1,8,20,52), DateTime (2022,3,1,8,22,38)),
SatellitePass (DateTime (2022,3,1,8,23,46), DateTime (2022,3,1,8,29,11)),
SatellitePass (DateTime (2022,3,1,9,6,6), DateTime (2022,3,1,9,10,32)),
SatellitePass (DateTime (2022,3,1,9,26,25), DateTime (2022,3,1,9,30,52)),
SatellitePass (DateTime (2022,3,1,9,58,33), DateTime (2022,3,1,10,3,46)),
SatellitePass (DateTime (2022,3,1,10,5,19), DateTime (2022,3,1,10,9,14)),
SatellitePass (DateTime (2022,3,1,10,22,0), DateTime (2022,3,1,10,27,16)),
SatellitePass (DateTime (2022,3,1,10,45,51), DateTime (2022,3,1,10,50,58)),
SatellitePass (DateTime (2022,3,1,11,39,11), DateTime (2022,3,1,11,43,31)),
SatellitePass (DateTime (2022,3,1,12,3,0), DateTime (2022,3,1,12,7,46)),
SatellitePass (DateTime (2022,3,1,12,27,28), DateTime (2022,3,1,12,30,6)),
SatellitePass (DateTime (2022,3,1,16,38,54), DateTime (2022,3,1,16,41,55)),
SatellitePass (DateTime (2022,3,1,17,33,57), DateTime (2022,3,1,17,38,29)),
SatellitePass (DateTime (2022,3,1,17,47,59), DateTime (2022,3,1,17,52,37)),
SatellitePass (DateTime (2022,3,1,17,55,59), DateTime (2022,3,1,17,59,27)),
SatellitePass (DateTime (2022,3,1,18,16,16), DateTime (2022,3,1,18,21,37)),
SatellitePass (DateTime (2022,3,1,18,56,51), DateTime (2022,3,1,19,0,21)),
SatellitePass (DateTime (2022,3,1,19,12,59), DateTime (2022,3,1,19,18,4)),
SatellitePass (DateTime (2022,3,1,19,27,22), DateTime (2022,3,1,19,32,6)),
SatellitePass (DateTime (2022,3,1,19,30,28), DateTime (2022,3,1,19,33,17)),
SatellitePass (DateTime (2022,3,1,19,47,52), DateTime (2022,3,1,19,52,42)),
SatellitePass (DateTime (2022,3,1,19,58,8), DateTime (2022,3,1,20,2,40)),
SatellitePass (DateTime (2022,3,1,20,15,3), DateTime (2022,3,1,20,19,56)),
SatellitePass (DateTime (2022,3,1,20,34,26), DateTime (2022,3,1,20,39,44)),
SatellitePass (DateTime (2022,3,1,21,9,35), DateTime (2022,3,1,21,10,18)),
SatellitePass (DateTime (2022,3,1,21,27,42), DateTime (2022,3,1,21,32,46)),
SatellitePass (DateTime (2022,3,1,21,55,13), DateTime (2022,3,1,22,0,29)),
SatellitePass (DateTime (2022,3,1,22,16,39), DateTime (2022,3,1,22,20,27))

    // TODO: add in more passes or calculate automatically
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
