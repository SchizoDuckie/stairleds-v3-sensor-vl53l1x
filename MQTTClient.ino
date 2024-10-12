#include "MQTTClient.h"

MQTTClient::MQTTClient(Config& config) : config(config), client(espClient) {}

void MQTTClient::setup() {
  client.setServer(config.mqttBroker, config.mqttPort);
}

void MQTTClient::handle() {
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
}

bool MQTTClient::isConnected() {
  return client.connected();
}

bool MQTTClient::reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  // If we're already connected, don't attempt reconnect
  if (client.connected()) {
    return true;
  }

  char clientId[23]; // Static client ID buffer
  snprintf(clientId, sizeof(clientId), "ESP8266Client-%04X", random(0xFFFF));

  if (client.connect(clientId)) {
    Serial.println("Connected to MQTT broker");
    return true;
  }
  return false;
}


bool MQTTClient::publish(const String& message) {
  if (!client.connected()) {
    return false;
  }
  return client.publish(config.mqttTopic, message.c_str());
}