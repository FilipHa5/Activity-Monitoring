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
    if (myFile) {
        refresh_date_and_time();
        String body_and_gps_data = String(analogRead(EMG)) + ',' + String(analogRead(ECG)) + ',' + String(analogRead(TEMPERATURE_PIN)) + ',' + String(gps.location.lat(), 6) +
            ',' + String(gps.location.lng(), 6) + ',' + String(gps.altitude.meters()) + ',' + String(gps.speed.kmph()) + ',' + String(gps.course.deg()) +
            ',' + String(gps.satellites.value()) + ',' + String(gps.hdop.value()) + ',' + String(year) + '-' + String(month) + '-' + String(day) +
            ',' + String(hour) + ':' + String(minute) + ':' + String(second);
        myFile.println(body_and_gps_data);
        Serial.println(body_and_gps_data);
    }
    myFile.close();
}

void log_body_data() {
    myFile = SD.open("GPS_data.txt", FILE_WRITE);
    int emg_arr[20];
    int ecg_arr[20];
    for (int i = 0; i < 20; i++) {
        emg_arr[i] = analogRead(EMG);
        ecg_arr[i] = analogRead(ECG);
        delay(1);
    }
    String body_data = String(analogRead(EMG)) + ',' + String(analogRead(ECG)) + ',';
    if (myFile) {
        for (int i = 0; i < 20; i++) {
            myFile.print(ecg_arr[i]);
            myFile.print(',');
            myFile.print(emg_arr[i]);
            myFile.print("\n");
        }
    }
    myFile.close();
}

void wait_till_serial_available() {
    while (!Serial.available()) {
        Serial.println("Waiting for serial. Logged body");
        log_body_data();
    }
    gps.encode(Serial.read());
    Serial.println("Available!, encoded");
}

/************************************************************
                           Loop
************************************************************/

//this version works, but make logs sequentially (with brakes) - so average sampling rate is too low

void loop() {
    static int counter = 0;

    if (counter == 0) {
        if ((Serial.available()) > 0) {
            if ((gps.encode(Serial.read()))) {
                log_full_data();
                counter++;
            }
        } else {
            wait_till_serial_available();
        }
        if (counter > 0) {
            for (int i = 0; i < 40; i++) {
                log_body_data();
            }
            counter = 0;
        }
    }
}
