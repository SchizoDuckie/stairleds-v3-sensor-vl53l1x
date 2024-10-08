// Sensor.cpp

#include "Sensor.h"
#include "Config.h"

void Sensor::setup() {
  Wire.begin();
  if (!vl53.begin(0x29, &Wire)) {
    Serial.println(F("Failed to boot VL53L1X"));
    while(1);
  }
  vl53.setTimingBudget(50);
  vl53.startRanging();
}

void Sensor::read() {
  if (vl53.dataReady()) {
    distance = vl53.distance();
    vl53.clearInterrupt();
  } else {
    Serial.println("Sensor data not ready");
  }
}

String Sensor::getData() {
  if (distance == -1) {
    return config.sensorName + "|Error: No distance data available";
  }
  return config.sensorName + "|Distance: " + String(distance);
}
