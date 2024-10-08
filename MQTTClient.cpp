// MQTTClient.cpp

#include "MQTTClient.h"
#include "Config.h"
#include <WiFiClient.h>

void MQTTClient::setup() {
  WiFiClient wifiClient;
  client.setClient(wifiClient);
  client.setServer(config.mqttBroker.c_str(), 1883);
}

void MQTTClient::handle() {
  client.loop();
}

void MQTTClient::reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(config.sensorName.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 2 seconds");
      delay(2000);
    }
  }
}

bool MQTTClient::isConnected() {
  return client.connected();
}

void MQTTClient::publish(const String& data) {
  if (client.publish(config.mqttTopic.c_str(), data.c_str())) {
    Serial.println("MQTT publish success");
  } else {
    Serial.println("MQTT publish failed");
  }
}
