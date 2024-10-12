#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

class Config {
public:
  void load();
  void save();

  char sensorName[32];      // Assuming a max length of 31 chars for the name
  char wifiSSID[32];        // Same for SSID
  char wifiPassword[64];    // Passwords are generally longer
  char mqttBroker[16];      // IP addresses or short domain names
  int mqttPort;
  char mqttTopic[64];       // Topics can be long

 // Getter methods for WiFi credentials
    const char* getWifiSSID() const;       // Getter for WiFi SSID
    const char* getWifiPassword() const;    // Getter for WiFi Password
};

extern Config config;

#endif