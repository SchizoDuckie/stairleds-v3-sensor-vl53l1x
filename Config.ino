#include "Config.h"
#include <EEPROM.h>
#include <ArduinoJson.h>

void Config::load() {
  EEPROM.begin(512);

  DynamicJsonDocument doc(512); 

  // Read EEPROM directly into JsonDocument without storing in String first
  DeserializationError error = deserializeJson(doc, EEPROM.getDataPtr(), 512);
  EEPROM.end();

  if (error) {
    Serial.println(F("Failed to parse config file"));
    return;
  }

  strlcpy(sensorName, doc["sensorName"] | "StairledSensor", sizeof(sensorName));
  strlcpy(wifiSSID, doc["wifiSSID"] | "", sizeof(wifiSSID));
  strlcpy(wifiPassword, doc["wifiPassword"] | "", sizeof(wifiPassword));
  strlcpy(mqttBroker, doc["mqttBroker"] | "10.0.0.1", sizeof(mqttBroker));
  mqttPort = doc["mqttPort"] | 1883;
  strlcpy(mqttTopic, doc["mqttTopic"] | "stairled/sensor1", sizeof(mqttTopic));
}



void Config::save() {
  DynamicJsonDocument doc(512);
  doc["sensorName"] = sensorName;
  doc["wifiSSID"] = wifiSSID;
  doc["wifiPassword"] = wifiPassword;
  doc["mqttBroker"] = mqttBroker;
  doc["mqttPort"] = mqttPort;
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


const char* Config::getWifiSSID() const {
    return wifiSSID;
}

const char* Config::getWifiPassword() const {
    return wifiPassword;
}