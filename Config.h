// Config.h

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

class Config {
public:
  void load();
  void save();

  String sensorName;
  String wifiSSID;
  String wifiPassword;
  String mqttBroker;
  String mqttTopic;
};

extern Config config;

#endif
