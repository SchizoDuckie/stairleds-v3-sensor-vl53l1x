#include "Sensor.h"
#include "Config.h"

extern Config config;

void Sensor::setup() {
  Serial.println(F("Sensor::setup: Initializing VL53L1X sensor..."));
  
  Wire.begin();
  delay(100);  // Increase delay to give more time for I2C to stabilize

  // Check if I2C device is responding
  Wire.beginTransmission(0x29);
  byte error = Wire.endTransmission();
  if (error != 0) {
    Serial.print(F("I2C device not responding. Error: "));
    Serial.println(error);
    Serial.println(F("Halting."));
    while (1) delay(10);
  }

  Serial.println(F("I2C device responding. Attempting to initialize VL53L1X..."));
  uint8_t attempts = 0;
  while (attempts < 5) {
    if (sensor.begin(0x29, &Wire)) {
      Serial.println(F("VL53L1X sensor initialized!"));
      break;
    }
    Serial.print(F("Error on init of VL sensor (attempt "));
    Serial.print(attempts + 1);
    Serial.print(F("): "));
    Serial.println(sensor.vl_status);
    delay(200);  // Increase delay between attempts
    attempts++;
  }

  if (attempts == 5) {
    Serial.println(F("Failed to initialize VL53L1X after 5 attempts. Halting."));
    while (1) delay(10);
  }

  Serial.print(F("Sensor ID: 0x"));
  Serial.println(sensor.sensorID(), HEX);

  Serial.println(F("Attempting to start ranging..."));
  if (!sensor.startRanging()) {
    Serial.print(F("Couldn't start ranging: "));
    Serial.println(sensor.vl_status);
    Serial.println(F("Halting."));
    while (1) delay(10);
  }

  // Set timing budget to 100 ms for more reliable readings
  sensor.setTimingBudget(100);

  Serial.println(F("VL53L1X sensor setup completed successfully"));
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
    return String(config.getSensorName()) + "|Error: No distance data available";
  }
  return String(config.getSensorName()) + "|Distance: " + String(distance);
}

int Sensor::getDistance() const {
  return distance;
}