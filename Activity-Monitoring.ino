#include "SD.h"

#include <SPI.h>

#include <TinyGPS++.h>

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
int day, month, year, hour, minute, second;

//float EMG_arr[100], ECG_arr[100];

// Objects
File myFile;
TinyGPSPlus gps;

/************************************************************
              Setup
************************************************************/

void
setup() {
  Serial.begin(9600);
  while (!Serial)
    delay(10);
  Serial.print("Initializing SD card...");
  if (!SD.begin(CS_SD)) {
    Serial.println("Initialization failed!");
    return 0;
  }
  Serial.println("Creating GPS_data.txt...");
  myFile = SD.open("GPS_data.txt", FILE_WRITE);
  Serial.println("File has been created");

  if (myFile) {
    myFile.println("EMG, ECG, T, Lat, Long, Alt, Date, Time\r\n");
    myFile.close();
  } else {
    Serial.println("error opening GPS_data.txt");
  }

  while (true) {
    if (millis() > 5000 && gps.charsProcessed() < 10) {
      Serial.println("GPS not detected, waiting 10 sec...");
      delay(10000);
    } else {
      break;
    }
  }
}

/************************************************************
            Obtain GPS data
************************************************************/
void
refresh_date_and_time() {
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

void
refresh_location() {
  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    altitude = gps.altitude.meters();
    Serial.print("Lat: ");
    Serial.print(latitude);
    Serial.print(" Lon: ");
    Serial.print(longitude);
    Serial.print(" Alt: \r\n");
    Serial.println(altitude);
  }
}

/************************************************************
            Log to file
************************************************************/

void
log_to_file() {
  static int ctrl = 100;
  char sep = ',';
  myFile = SD.open("GPS_data.txt", FILE_WRITE);
  // Order: Muscle, Hearth, Temp, Latitude, Longitude, Altitude, Date, Time
  if (myFile) {
    myFile.print(analogRead(EMG));
    myFile.print(sep);
    myFile.print(analogRead(ECG));
    Serial.println("logged EMG and ECG data");
    ctrl--;
    if (myFile && ctrl < 0) {
      refresh_location();
      refresh_date_and_time();
      myFile.print(sep);
      myFile.print(analogRead(TEMPERATURE_PIN));
      myFile.print(sep);
      myFile.print(latitude, 6);
      myFile.print(sep);
      myFile.print(longitude, 6);
      myFile.print(sep);
      myFile.print(altitude, 4);
      myFile.print(sep);
      myFile.print(year);
      myFile.print('-');
      myFile.print(month);
      myFile.print('-');
      myFile.print(day);
      myFile.print(sep);
      myFile.print(hour);
      myFile.print(':');
      myFile.print(minute);
      myFile.print(':');
      myFile.print(second);
      Serial.println("logged FULL data");
      ctrl = 100;
    }
    myFile.print("\r\n");
    myFile.close();
  }
}
/************************************************************
              Loop
************************************************************/

void
loop() {
    if ((Serial.available() > 0) && (gps.encode(Serial.read()))) {
      log_to_file();
    }