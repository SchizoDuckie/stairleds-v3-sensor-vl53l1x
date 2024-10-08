// MQTTClient.h

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>
#include "Config.h"

class MQTTClient {
public:
  void setup();
  void handle();
  void reconnect();
  bool isConnected();
  void publish(const String& data);

private:
  PubSubClient client;
};

#endif
