// Config.cpp

#include "Config.h"
#include <EEPROM.h>
#include <ArduinoJson.h>

void Config::load() {
  EEPROM.begin(512);
  String jsonConfig;
  for (int i = 0; i < 512; i++) {
    jsonConfig += char(EEPROM.read(i));
  }
  EEPROM.end();

  DynamicJsonDocument doc(512);
  deserializeJson(doc, jsonConfig);

  sensorName = doc["sensorName"] | "StairledSensor";
  wifiSSID = doc["wifiSSID"] | "";
  wifiPassword = doc["wifiPassword"] | "";
  mqttBroker = doc["mqttBroker"] | "192.168.1.100";
  mqttTopic = doc["mqttTopic"] | "stairled/sensor1";
}

void Config::save() {
  DynamicJsonDocument doc(512);
  doc["sensorName"] = sensorName;
  doc["wifiSSID"] = wifiSSID;
  doc["wifiPassword"] = wifiPassword;
  doc["mqttBroker"] = mqttBroker;
  doc["mqttTopic"] = mqttTopic;

  String jsonConfig;
  serializeJson(doc, jsonConfig);

  EEPROM.begin(512);
  for (unsigned int i = 0; i < jsonConfig.length(); i++) {
    EEPROM.write(i, jsonConfig[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}
