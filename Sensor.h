#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_VL53L1X.h"

#define IRQ_PIN 2
#define XSHUT_PIN 3

Adafruit_VL53L1X sensor = Adafruit_VL53L1X(XSHUT_PIN, IRQ_PIN);

class Sensor {
public:
  void setup();
  bool read();
  String getData();
  int getDistance() const;

private:
  
  int distance = -1;
};

#endif