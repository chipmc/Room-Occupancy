// Time of Flight Sensor Class
// Author: Chip McClelland
// Date: May 2023
// License: GPL3
// This is the class for the ST Micro VL53L1X Time of Flight Sensor
// We are using the Sparkfun library which has some shortcomgings
// - It does not implement distance mode medium
// - It does not give access to the factory calibration of the optical center

#include "Arduino.h"
#include "ErrorCodes.h"
#include "TofSensorConfig.h"
#include "PeopleCounterConfig.h"
#include "TofSensor.h"

uint8_t opticalCenters[2] = {FRONT_ZONE_CENTER,BACK_ZONE_CENTER}; 
int zoneDistances[2] = {0,0};
int occupancyState = 0;      // This is the current occupancy state (occupied or not, zone 1 (ones) and zone 2 (twos)

TofSensor *TofSensor::_instance;

// [static]
TofSensor &TofSensor::instance() {
  if (!_instance) {
      _instance = new TofSensor();
  }
  return *_instance;
}

TofSensor::TofSensor() {
}

TofSensor::~TofSensor() {
}

SFEVL53L1X myTofSensor;

void TofSensor::setup(){
  if(myTofSensor.begin() != 0){
    Log.infoln("Sensor error reset in 10 seconds");
    delay(10000);
    Log.infoln("Should be Resetting");
    // System.reset();
  }
  else Log.infoln("Sensor init successfully");
  
  // Here is where we set the device properties
  myTofSensor.setDistanceModeLong();
  myTofSensor.setSigmaThreshold(40);        // Default is 45 - this will make it harder to get a valid result - Range 1 - 16383
  myTofSensor.setSignalThreshold(1000);     // Default is 1500 raising value makes it harder to get a valid results- Range 1-16383
  myTofSensor.setTimingBudgetInMs(33);      // Was 20mSec

  while (TofSensor::loop() == SENSOR_BUFFRER_NOT_FULL) {delay(10);}; // Wait for the buffer to fill up
  Log.infoln("Buffer is full - will now calibrate");

  if (TofSensor::performCalibration()) Log.infoln("Calibration Complete");
  else {
    Log.infoln("Initial calibration failed - wait 10 secs and reset");
    delay(10000);
    // System.reset();
    Log.infoln("Should be Resetting");
  }

  // myTofSensor.setDistanceModeShort();                     // Once initialized, we are focused on the top half of the door

}

bool TofSensor::performCalibration() {
  TofSensor::loop();                  // Get the latest values
  if (occupancyState != 0){
    Log.infoln("Target zone not clear - will wait ten seconds and try again");
    delay(10000);
    TofSensor::loop();
    if (occupancyState != 0) return false;
  }
  Log.infoln("Target zone is clear with zone1 at %imm and zone2 at %imm",zoneDistances[0],zoneDistances[1]);
  return true;
}

int TofSensor::loop(){                         // This function will update the current distance / occupancy for each zone.  It will return true if occupancy changes                    
  int oldOccupancyState = occupancyState;
  occupancyState = 0;
  static uint16_t Distances[2][DISTANCES_ARRAY_SIZE]; // This is the array of distances for each zone
  static uint8_t DistancesTableSize[2] = {0,0};
  uint16_t MinDistance;

  unsigned long startedRanging;

  for (byte zone = 0; zone < 2; zone++){
    myTofSensor.stopRanging();
    myTofSensor.clearInterrupt();
    myTofSensor.setROI(ROWS_OF_SPADS,COLUMNS_OF_SPADS,opticalCenters[zone]);
    delay(1);
    myTofSensor.startRanging();

    startedRanging = millis();
    while(!myTofSensor.checkForDataReady()) {
      if (millis() - startedRanging > SENSOR_TIMEOUT) {
        Log.infoln("Sensor Timed out");
        return SENSOR_TIMEOUT_ERROR;
      }
    }

    #if DEBUG_COUNTER
    Log.infoln("Zone%d (%dx%d %d SPADs with optical center %d) = %imm",zone+1,myTofSensor.getROIX(), myTofSensor.getROIY(), myTofSensor.getSpadNb(),opticalCenters[zone],zoneDistances[zone]);
    delay(500);
    #endif

    // Add just picked distance to the table of the corresponding zone
    if (DistancesTableSize[zone] < DISTANCES_ARRAY_SIZE) {
      Distances[zone][DistancesTableSize[zone]] = myTofSensor.getDistance();;
      DistancesTableSize[zone] ++;
      return SENSOR_BUFFRER_NOT_FULL;
    }
    else {
      for (int i=1; i<DISTANCES_ARRAY_SIZE; i++) {
        Distances[zone][i-1] = Distances[zone][i];
        Distances[zone][DISTANCES_ARRAY_SIZE-1] = myTofSensor.getDistance();
      }
    }
    
    // pick up the min distance
    MinDistance = Distances[zone][0];
    if (DistancesTableSize[zone] >= 2) {
      for (int i=1; i<DistancesTableSize[zone]; i++) {
        if (Distances[zone][i] < MinDistance)
          MinDistance = Distances[zone][i];
      }
    }

    zoneDistances[zone] = MinDistance;
    bool occupied = ((MinDistance < PERSON_THRESHOLD) && (MinDistance > DOOR_THRESHOLD));
    occupancyState += occupied * (zone + 1);
  }

  #if PEOPLECOUNTER_DEBUG
  if (occupancyState != oldOccupancyState) Log.infoln("Occupancy state changed from %d to %d (%imm / %imm)", oldOccupancyState, occupancyState, zoneDistances[0], zoneDistances[1]);
  #endif

  return (occupancyState != oldOccupancyState);     // Let us know if the occupancy state has changed
}

int TofSensor::getZone1() {
  return zoneDistances[0];
}

int TofSensor::getZone2() {
  return zoneDistances[1];
}

int TofSensor::getOccupancyState() {
  return occupancyState;
}



