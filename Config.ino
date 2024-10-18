#include "Config.h"
#include <EEPROM.h>

#define CONFIG_VERSION 1
#define CONFIG_START 0

void Config::load() {
  EEPROM.begin(512);
  
  // Check if the stored config version matches
  uint8_t version = EEPROM.read(CONFIG_START);
  if (version == CONFIG_VERSION) {
    Serial.println(F("Loading configuration"));
    for (unsigned int t = 0; t < sizeof(data); t++) {
      *((char*)&data + t) = EEPROM.read(CONFIG_START + 1 + t);
    }
  } else {
    Serial.println(F("Config version mismatch, using defaults"));
    // Set default values
    String defaultChipName = String(ESP.getChipId(), HEX);
    strlcpy(data.sensorName, defaultChipName.c_str(), sizeof(data.sensorName));
    data.wifiSSID[0] = '\0';
    data.wifiPassword[0] = '\0';
    strlcpy(data.mqttBroker, "10.0.0.1", sizeof(data.mqttBroker));
    data.mqttPort = 1883;
    strlcpy(data.mqttTopic, "stairled/", sizeof(data.mqttTopic));
  }
  EEPROM.end();
}

void Config::save() {
  EEPROM.begin(512);
  EEPROM.write(CONFIG_START, CONFIG_VERSION);
  for (unsigned int t = 0; t < sizeof(data); t++) {
    EEPROM.write(CONFIG_START + 1 + t, *((char*)&data + t));
  }
  EEPROM.commit();
  EEPROM.end();
  Serial.println(F("Configuration saved"));
}

const char* Config::getSensorName() const {
  return data.sensorName;
}

void Config::setSensorName(const char* name) {
  strlcpy(data.sensorName, name, sizeof(data.sensorName));
}

const char* Config::getWifiSSID() const {
  return data.wifiSSID;
}

const char* Config::getWifiPassword() const {
  return data.wifiPassword;
}

void Config::setWifiCredentials(const char* ssid, const char* password) {
  strlcpy(data.wifiSSID, ssid, sizeof(data.wifiSSID));
  strlcpy(data.wifiPassword, password, sizeof(data.wifiPassword));
}

const char* Config::getMqttBroker() const {
  return data.mqttBroker;
}

void Config::setMqttBroker(const char* broker) {
  strlcpy(data.mqttBroker, broker, sizeof(data.mqttBroker));
}

uint16_t Config::getMqttPort() const {
  return data.mqttPort;
}

void Config::setMqttPort(uint16_t port) {
  data.mqttPort = port;
}

const char* Config::getMqttTopic() const {
  return data.mqttTopic;
}

void Config::setMqttTopic(const char* topic) {
  
  strlcpy(data.mqttTopic, topic, sizeof(data.mqttTopic));
}

// Add other getter and setter methods as needed

