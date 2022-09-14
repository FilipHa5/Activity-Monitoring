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
    Serial.println("Initialization failed!");
    while (1);
  }
  Serial.println("Initialization done");
  Serial.println("Creating GPS_data.txt...");
  myFile = SD.open("GPS_data.txt", FILE_WRITE);
   Serial.println("File has been created");
  
  if (myFile) {
    myFile.println("EMG, ECG, T, Lat, Long, Alt, Date, Time\r\n");
    myFile.close();
  } 
  else
    Serial.println("error opening GPS_data.txt");

 SerialGPS.begin(9600);
}

/************************************************************
                        Obtain GPS data
 ************************************************************/
String get_date_and_time() {
    if (gps.date.isValid()) {
        month = gps.date.month();
        day = gps.date.day();
        year = gps.date.year();
        Date = day + "-" + month + "-" + year;

        Serial.print("Date ");
        Serial.println(Date);
    }
    if (gps.time.isValid()) {
        if (gps.time.hour() < 10) ;
            hour = gps.time.hour();
        if (gps.time.minute() < 10);
            minute = gps.time.minute();
        if (gps.time.second() < 10) ;
            second = gps.time.second();
        Time = hour + ":" + minute + ":" + second;

        Serial.print(" Time \r\n");
        Serial.println(Time);
    }
  String DateTime = Date + "," + Time;
  return DateTime;
}

void refresh_location() {
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

void log_to_file() {
    static int ctrl = 100000;
    char sep = ",";
    myFile = SD.open("GPS_data.txt", FILE_WRITE);
    //Order: Muscle, Hearth, Temp, Latitude, Longitude, Altitude, Date, Time
    if (myFile){
        myFile.print(analogRead(EMG));
        myFile.print(sep);
        myFile.print(analogRead(ECG));
        myFile.print(sep);
        myFile.print(analogRead(Temperature));
        Serial.println("logged body data");
        Serial.println(ctrl);
        --ctrl;

        if ((ctrl <= 0)) {
            refresh_location();
            myFile.print(sep);
            myFile.print(latitude, 6);
            myFile.print(sep);
            myFile.print(longitude, 6);
            myFile.print(sep);
            myFile.print(altitude, 4);
            myFile.print(sep);
            myFile.print(get_date_and_time());
            Serial.println("logged GPS data");
            ctrl = 100000;
        }

        myFile.print("\r\n");
        myFile.close();
    }
}

/************************************************************
                            Loop
 ************************************************************/
void loop() {
  if (millis() > 5000 && gps.charsProcessed() < 10){
    Serial.println("GPS not detected, waiting 10 sec...");
    delay (10000);
  }

  if ((SerialGPS.available() > 0) && (gps.encode(SerialGPS.read())))
      log_to_file();
}
