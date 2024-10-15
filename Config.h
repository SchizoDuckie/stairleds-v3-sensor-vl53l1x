#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

class Config {
public:
  void load();
  void save();
  const char* getSensorName() const;
  void setSensorName(const char* name);
  const char* getWifiSSID() const;
  const char* getWifiPassword() const;
  void setWifiCredentials(const char* ssid, const char* password);
  const char* getMqttBroker() const;
  void setMqttBroker(const char* broker);
  uint16_t getMqttPort() const;
  void setMqttPort(uint16_t port);
  const char* getMqttTopic() const;
  void setMqttTopic(const char* topic);

private:
  struct ConfigData {
    char sensorName[32];
    char wifiSSID[33];
    char wifiPassword[65];
    char mqttBroker[64];
    uint16_t mqttPort;
    char mqttTopic[64];
  } data;
};

extern Config config;

#endif
