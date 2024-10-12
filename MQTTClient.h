#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "Config.h"

class MQTTClient {
public:
  MQTTClient(Config& config);
  void setup();
  void handle();
  bool isConnected();
  bool reconnect();
  bool publish(const String& message);

private:
  Config& config;
  WiFiClient espClient;
  PubSubClient client;
  unsigned long lastReconnectAttempt = 0;
  const unsigned long RECONNECT_INTERVAL = 5000; // 5 seconds between reconnect attempts
};

#endif