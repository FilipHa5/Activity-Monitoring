#include "SD.h"
#include <SPI.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>


/************************************************************
				Variables and objects declaration
 ************************************************************/

//Pins
constexpr unsigned int RX_PIN = 8;
constexpr unsigned int TX_PIN = 9;
constexpr unsigned int CS_SD = 10;
constexpr unsigned int EMG = A0;
constexpr unsigned int ECG = A1;
constexpr unsigned int TEMPERATURE_PIN = A2;

// Other
float latitude;
float longitude;
float altitude;
float EMG_Value;
float ECG_Value;
float Temperature_Value;

String day, month, year, hour, minute, second, Date, Time, Data, DateTime;

//Objects
File myFile;
TinyGPSPlus gps;
SoftwareSerial SerialGPS(RX_PIN, TX_PIN);

/************************************************************
							Setup
 ************************************************************/

void setup() {
	Serial.begin(9600);
	while (!Serial) 
		delay(10);
		Serial.print("Initializing SD card...");
	if (!SD.begin(CS_SD)) {
		Serial.println("Initialization failed!");
		return 0;
	}
	Serial.println("Initialization done");
	Serial.println("Creating GPS_data.txt...");
	myFile = SD.open("GPS_data.txt", FILE_WRITE);
	Serial.println("File has been created");
	
	if (myFile) {
		myFile.println("EMG, ECG, T, Lat, Long, Alt, Date, Time\r\n");
		myFile.close();
	} else {
		Serial.println("error opening GPS_data.txt");
	}

	SerialGPS.begin(9600);
	
	while(true) {
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

/************************************************************
						Log to file
 ************************************************************/

void log_to_file() {
	static int ctrl = 100;
	char sep = ',';
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

		if (ctrl <= 0) {
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
			ctrl = 100;
		}

		myFile.print("\r\n");
		myFile.close();
	}
}

/************************************************************
							Loop
 ************************************************************/

void loop() {
  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read()))
      log_to_file();
  }
}
