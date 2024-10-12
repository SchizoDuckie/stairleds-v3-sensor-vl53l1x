#include "Sensor.h"
#include "Config.h"

extern Config config;



void Sensor::setup() {
  Wire.begin();

   Wire.begin();
  if (! sensor.begin(0x29, &Wire)) {
    Serial.print(F("Error on init of VL sensor: "));
    Serial.println(sensor.vl_status);
    while (1)       delay(10);
  }
  Serial.println(F("VL53L1X sensor OK!"));

  Serial.print(F("Sensor ID: 0x"));
  Serial.println(sensor.sensorID(), HEX);

  if (! sensor.startRanging()) {
    Serial.print(F("Couldn't start ranging: "));
    Serial.println(sensor.vl_status);
    while (1)       delay(10);
  }
  Serial.println(F("VL53L1X sensor initialized successfully"));
}

bool Sensor::read() {
  if (sensor.dataReady()) {
    distance = sensor.distance();
    //Serial.printf("Sensor measurement: %u\n", distance);
    sensor.clearInterrupt();
    return true;
  }
  return false;
}

String Sensor::getData() {
  if (distance < 0) {
    return String(config.sensorName) + "|Error: No distance data available";
  }
  return String(config.sensorName) + "|Distance: " + String(distance);
}

int Sensor::getDistance() const {
  return distance;
}