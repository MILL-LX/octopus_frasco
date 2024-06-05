
/* ----------------------------------------------------------------------------

# Simple Motion Sensing And Data Logging Module

Tiago Rorke - MILL Makers in Little Lisbon
May 2024

## Libraries

- SD Library for Arduino - https://github.com/arduino-libraries/SD
- SparkFun ICM-20948 Arduino Library - https://github.com/sparkfun/SparkFun_ICM-20948_ArduinoLibrary/
   - part of this code is adapted from Example1_Basics

## Hardware

- Adafruit Feather M0 Adalogger
- SparkFun 9DoF IMU Breakout - ICM-20948
- Generic 1000mAh lipo battery connected to feather

 --------------------------------------------------------------------------- */

// SD Card
#include <SPI.h>
#include <SD.h>
const int chipSelect = 4;

// 9DoF IMU
#include "ICM_20948.h"
// On the SparkFun 9DoF IMU breakout the default i2c address is 1, and when the ADR jumper is closed the value becomes 0
#define IMU_ADDR 1
ICM_20948_I2C imu; // Create an ICM_20948_I2C object

// Data logging parameters
const int min_sensor_refresh_delay = 30; // minimum delay between sensor readings


// ------------------------------------------------------------------------- //

void setup() {

   Serial.begin(115200);
   pinMode(LED_BUILTIN, OUTPUT);
   digitalWrite(LED_BUILTIN, LOW);
   delay(1000);

   // Setup SD card -------------------------------- //

   Serial.print("Initializing SD card...");
   if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
      // if there is a problem with the SD card, blink the built-in quickly LED 2 times in loop
      while (1) {
         blink(2, 200);
         delay(1000);
      }
   }
   Serial.println("card initialized.");

   // Setup IMU ------------------------------------ //

   Wire.begin();
   Wire.setClock(400000);
   //imu.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial
   bool imu_initialized = false;

   while (!imu_initialized) {
      imu.begin(Wire, IMU_ADDR);
      Serial.print(F("Initialization of the sensor returned: "));
      Serial.println(imu.statusString());

      // if there is a problem initialising the sensor, blink the built-in quickly LED 3 times
      if (imu.status != ICM_20948_Stat_Ok) {
         Serial.println("Trying again...");
         blink(3, 200);
         // wait 1s before retrying
         delay(1000);
      } else {
         imu_initialized = true;
      }
   }

}


// ------------------------------------------------------------------------- //

void loop() {

   // If the sensor is ready to update, get updated readings
   if (imu.dataReady()) {
/*
      String data = "";
      data += (millis());
      data += ",";
      for (int analogPin = 0; analogPin < 3; analogPin++) {
         int sensor = analogRead(analogPin);
         data += String(sensor);
         if (analogPin < 2) {
            data += ",";
         }
      }
*/
      imu.getAGMT();

      // choose between scaled values and raw values
      String data = scaledAGMT(&imu);      // get scaled values from sensor
      //String data = rawAGMT(myICM.agmt); // get raw values from sensor

      // Log data to SD card
      logData(data);

      delay(min_sensor_refresh_delay);

   // otherwise wait 0.5s before retrying to read from the sensor
   } else {
      Serial.println("Waiting for data");
      delay(500);
   }

}


// ------------------------------------------------------------------------- //

void logData(String dataString) {

   // blink the built-in LED while saving data to SD card
   digitalWrite(LED_BUILTIN, HIGH);

   File dataFile = SD.open("datalog.csv", FILE_WRITE);

   // if the file is ok, append the new data
   if (dataFile) {
      // if the file is new, add column headers
      if(dataFile.size() <= 0) {
         dataFile.println(columnHeaders());
      }
      dataFile.println(dataString);
      dataFile.close();
      Serial.println(dataString);

   // otherwise if there is a problem, blink the LED quickly 5 times
   } else {
      Serial.println("error opening datalog.csv");
      digitalWrite(LED_BUILTIN, LOW);
      blink(5, 200);
      // wait 1 second before retrying
      delay(1000);
   }

   digitalWrite(LED_BUILTIN, LOW);
}


// Utility function to blink built-in LED
void blink(int n, int d) {
   for (int i=0; i<n; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(d);
      digitalWrite(LED_BUILTIN, LOW);
      delay(d);
   }
}


String columnHeaders() {
   String n = "";
   n += ("time");
   n += ",";
   n += ("acc_x");
   n += ",";
   n += ("acc_y");
   n += ",";
   n += ("acc_z");
   n += ",";
   n += ("gyr_x");
   n += ",";
   n += ("gyr_y");
   n += ",";
   n += ("gyr_z");
   n += ",";
   n += ("mag_x");
   n += ",";
   n += ("mag_y");
   n += ",";
   n += ("mag_z");
   n += ",";
   n += ("tmp");
   return n;
}

String rawAGMT(ICM_20948_AGMT_t agmt) {
   String n = "";
   n += (millis());
   n += ",";
   n += (agmt.acc.axes.x);
   n += ",";
   n += (agmt.acc.axes.y);
   n += ",";
   n += (agmt.acc.axes.z);
   n += ",";
   n += (agmt.gyr.axes.x);
   n += ",";
   n += (agmt.gyr.axes.y);
   n += ",";
   n += (agmt.gyr.axes.z);
   n += ",";
   n += (agmt.mag.axes.x);
   n += ",";
   n += (agmt.mag.axes.y);
   n += ",";
   n += (agmt.mag.axes.z);
   n += ",";
   n += (agmt.tmp.val);
   return n;
}

String scaledAGMT(ICM_20948_I2C *sensor) {
   String n = "";
   n += (millis());
   n += ",";
   n += (sensor->accX());
   n += ",";
   n += (sensor->accY());
   n += ",";
   n += (sensor->accZ());
   n += ",";
   n += (sensor->gyrX());
   n += ",";
   n += (sensor->gyrY());
   n += ",";
   n += (sensor->gyrZ());
   n += ",";
   n += (sensor->magX());
   n += ",";
   n += (sensor->magY());
   n += ",";
   n += (sensor->magZ());
   n += ",";
   n += (sensor->temp());
   return n;
}