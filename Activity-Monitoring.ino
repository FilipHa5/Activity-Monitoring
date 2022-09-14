#include "SD.h"
#include <SPI.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>


/************************************************************
                Variables and objects declaration
 ************************************************************/

//Pins
const unsigned int RXPin = 8;
const unsigned int TXPin = 9;
const unsigned int ChipSelect_SD = 10;
const unsigned int EMG = A0;
const unsigned int ECG = A1;
const unsigned int Temperature = A2;

// Other
int ctrl = 100000;
double latitude;
double longitude;
double altitude;
float EMG_Value;
float ECG_Value;
float Temperature_Value;

//Objects
File myFile;
TinyGPSPlus gps;

SoftwareSerial SerialGPS(RXPin, TXPin);

String Latitude, Longitude, Altitude, day, month, year,
       hour, minute, second, Date, Time, Data, DateTime;

/************************************************************
                            Setup
 ************************************************************/
void setup() {
  Serial.begin(9600);
    while (!Serial);

  Serial.print("Initializing SD card...");

  if (!SD.begin(ChipSelect_SD)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  Serial.println("Creating GPS_data.txt...");
  myFile = SD.open("GPS_data.txt", FILE_WRITE);
 
  if (myFile) {
    myFile.println( "Latitude, Longitude, Altitude, Date and Time, Muscle, Hearth, Temperature(R) \r\n");
    myFile.close();
  }
  else
    Serial.println("error opening GPS_data.txt");

 SerialGPS.begin(9600);
}

/************************************************************
                        Obtain GPS data
 ************************************************************/
String obtain_date_and_time() {
    if (gps.date.isValid()) {
        month = gps.date.month();
        day = gps.date.day();
        year = gps.date.year();
        Date = day + "-" + month + "-" + year;

        Serial.print("Date ");
        Serial.println(Date);
    }
    else
      Serial.println("Invalid date");
    if (gps.time.isValid()) {
        if (gps.time.hour() < 10) ;
            hour = gps.time.hour();
        if (gps.time.minute() < 10);
            minute = gps.time.minute();
        if (gps.time.second() < 10) ;
            second = gps.time.second();
        Time = hour + ":" + minute + ":" + second;

        Serial.print("Time ");
        Serial.println(Time);
    }
    else
      Serial.println("Invalid Time");
  String DateTime = Date + "," + Time;
  return DateTime;
}

void obtain_location() {
  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    altitude = gps.altitude.meters();

    Serial.print(latitude);
    Serial.print(longitude);
    Serial.println(altitude);
  }
  else
      Serial.println("Invalid Location");
}

void log_to_file() {
    char sep = ",";
    myFile = SD.open("GPS_data.txt", FILE_WRITE);
    //Order: Latitude, Longitude, Altitude, Date and Time, Muscle, Hearth, Temp
    if ((myFile) && (ctrl <= 0)) {
        obtain_location();
        obtain_date_and_time();

        myFile.print(latitude, 6);
        myFile.print(sep);
        myFile.print(longitude, 6);
        myFile.print(sep);
        myFile.print(altitude, 4);
        myFile.print(sep);
        myFile.print(obtain_date_and_time());
        myFile.print(sep);
        myFile.print(analogRead(EMG));
        myFile.print(sep);
        myFile.print(analogRead(ECG));
        myFile.print(sep);
        myFile.print(analogRead(Temperature));
        myFile.print("\r\n");
        myFile.close();
        ctrl = 100000;
    }
    if ((myFile) && (ctrl > 0)) {
        myFile.print(sep);
        myFile.print(sep);
        myFile.print(sep);
        myFile.print(sep);
        myFile.print(analogRead(EMG));
        myFile.print(sep);
        myFile.print(analogRead(ECG));
        myFile.print(sep);
        myFile.print(analogRead(Temperature));
        myFile.print("\r\n");
        myFile.close();
        --ctrl;

    }
}

/************************************************************
                            Loop
 ************************************************************/
void loop() {
  while (SerialGPS.available() > 0)
    if (gps.encode(SerialGPS.read()))
      log_to_file();

  if (millis() > 5000 && gps.charsProcessed() < 10){
    Serial.println("GPS NOT DETECTED!");
    while(true);
  }
}

