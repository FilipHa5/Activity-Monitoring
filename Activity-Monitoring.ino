#include <SPI.h>

#include <TinyGPS++.h>

#include "SD.h"

/************************************************************
           Variables and objects declaration
************************************************************/

// Pins
constexpr unsigned int RX_PIN = 8;
constexpr unsigned int TX_PIN = 9;
constexpr unsigned int CS_SD = 10;
constexpr unsigned int EMG = A0;
constexpr unsigned int ECG = A1;
constexpr unsigned int TEMPERATURE_PIN = A2;

// Other
float latitude, longitude, altitude;
String day, month, year, hour, minute, second, speed, sats, course, hdop;

// Objects
File myFile;
TinyGPSPlus gps;

/************************************************************
                      Setup functions
************************************************************/
int init_SD() {
  if (!SD.begin(CS_SD)) {
    Serial.println("Could not begin SD");
    return 0;
  } else {
    myFile = SD.open("GPS_data.txt", FILE_WRITE);
    if (myFile) {
      myFile.println("EMG, ECG, T, Lat, Long, Alt[m], Speed[km/k], Course, Sats, hdop, Date, Time\r");
      myFile.close();
      return 1;
    } else {
      Serial.println("Error opening GPS_data.txt");
      return 0;
    }
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10000);
  while (true) {
    if (millis() > 5000 && gps.charsProcessed() < 10) {
      Serial.println("GPS not detected, waiting 10 sec...");
      delay(10000);
    } else {
      break;
    }
  }

  while (true) {
    Serial.println("Initializing SD card...");
    if (init_SD() == 0) {
      Serial.println("Initialization failed!");
      delay(10000);
    } else {
      Serial.println("Initialization done, file has been created");
      break;

    }
  }
}

/************************************************************
                     Obtain GPS data
************************************************************/
void refresh_date_and_time() {
  if (gps.date.isValid()) {
    month = gps.date.month();
    day = gps.date.day();
    year = gps.date.year();
  }
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10)
    ;
    hour = gps.time.hour();
    if (gps.time.minute() < 10)
    ;
    minute = gps.time.minute();
    if (gps.time.second() < 10)
    ;
    second = gps.time.second();
  }
}

void log_full_data() {
  myFile = SD.open("GPS_data.txt", FILE_WRITE);
  String body_data = String(analogRead(EMG)) + ',' + String(analogRead(ECG)) + ',';
  if (myFile) {
    myFile.print("\r\n");
    myFile.print(body_data);
    Serial.println(body_data);
    refresh_date_and_time();
    String temp_and_gps_data = String(analogRead(TEMPERATURE_PIN)) + ',' + String(gps.location.lat(), 6) +
      ',' + String(gps.location.lng(), 6) + ',' + String(gps.altitude.meters()) + ',' + String(gps.speed.kmph()) + ',' + String(gps.course.deg()) +
      ',' + String(gps.satellites.value()) + ',' + String(gps.hdop.value()) + ',' + String(year) + '-' + String(month) + '-' + String(day) +
      ',' + String(hour) + ':' + String(minute) + ':' + String(second);
    myFile.print(temp_and_gps_data);
    Serial.println(temp_and_gps_data);
  }
  myFile.close();
}

void log_body_data() {
  myFile = SD.open("GPS_data.txt", FILE_WRITE);
  String body_data = String(analogRead(EMG)) + ',' + String(analogRead(ECG)) + ',';
  if (myFile) {
    myFile.print("\r\n");
    myFile.print(body_data);
    Serial.println(body_data);
  }
  myFile.close();
}

/************************************************************
                           Loop
************************************************************/

//this version works, but make logs sequentially (with brakes) - so average sampling rate is too low

void loop() {
  static int counter = 200;
  if ((Serial.available() > 0) && (gps.encode(Serial.read()))) {
    log_body_data();
    counter--;
    if (counter < 0) {
      log_full_data();
      counter = 200;
    }
   //else {...} here makes, that only else is executed. 
   //Moving (Serial.available() > 0) && (gps.encode(Serial.read())) to functions or other places in code makes that it doesn't work - logs only time or nothing
  }
}
