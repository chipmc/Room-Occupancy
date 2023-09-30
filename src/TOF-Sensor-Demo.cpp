/*
  TOF Based Occupancy Counter - Demo Code
  By: Chip McClelland
  May 2023
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SparkFun labored with love to create the library used in this sketch. Feel like supporting open source hardware? 
  Buy a board from SparkFun! https://www.sparkfun.com/products/14667

  Also, I got early inspiration from a project by Jan - https://github.com/BasementEngineering/PeopleCounter
  But, I needed to change the logic to better match my way of thinking.  Thank you Jan for getting me started!

  This example proves out some of the TOF sensor capabilitites we need to validate:
  - Abiltiy to Sleep and wake on range interrupt
  - Once awake, to determine if an object is moving parallel, toward or away from the sensor
  - Ability to deal with edge cases: Door closings, partial entries, collisions, chairs, hallway passings, etc.
*/

// v1.00 - Moving Particle code over to PlatformIO for the new LoRA Node
// v1.01 - Ported the code over to PlatfirmIO and the Adafruit Feather M0


#include "Arduino.h"
#include "ArduinoLog.h"
#include <Wire.h>
#include "pinout.h"
#include "stsLED.h"
#include "ErrorCodes.h"
#include "TofSensor.h"
#include "PeopleCounter.h"

char statusMsg[64] = "Startup Complete.  Running version 2.03";

void setup(void)
{
  Wire.begin();
  Log.begin(LOG_LEVEL_TRACE, &Serial);
  delay(3000);                              // Gives serial time to connect

  Log.infoln("Starting TOF Sensor Demo");

  gpio.setup();
  stsLED::instance().setup(gpio.STATUS);
  TofSensor::instance().setup();
  PeopleCounter::instance().setup();
  PeopleCounter::instance().setCount(1);

  Log.infoln(statusMsg);
}

unsigned long lastLedUpdate = 0;

void loop(void)
{
  if( (millis() - lastLedUpdate) > 1000 ){
    digitalWrite(gpio.STATUS,!digitalRead(gpio.STATUS));
    lastLedUpdate = millis();
  }

  if (TofSensor::instance().loop()) {         // If there is new data from the sensor
    PeopleCounter::instance().loop();         // Then check to see if we need to update the counts
  }
}
