// Sensor.h

#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_VL53L1X.h"

class Sensor {
public:
  void setup();
  void read();
  String getData();

private:
  Adafruit_VL53L1X vl53;
  int distance = -1; // Initialize to -1 to indicate no distance read yet
};

#endif
