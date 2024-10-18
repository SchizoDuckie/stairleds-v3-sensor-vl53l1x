#include "MQTTClient.h"

MQTTClient::MQTTClient(Config& config) : config(config), client(espClient) {}

void MQTTClient::setup()
{
  Serial.print(F("Setting up MQTT client with broker: "));
  Serial.print(config.getMqttBroker());
  Serial.print(F(" on port: "));
  Serial.println(config.getMqttPort());
  client.setServer(config.getMqttBroker(), config.getMqttPort());

  // Set the MQTT topic to include the sensor name
  mqttTopic = String(config.getMqttTopic()) + "/" + config.getSensorName();
}

void MQTTClient::handle()
{
  if (!isConnected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > RECONNECT_INTERVAL)
    {
      lastReconnectAttempt = now;
      if (reconnect())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    client.loop();
  }
}

bool MQTTClient::isConnected()
{
  return client.connected();
}

bool MQTTClient::reconnect()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("MQTT reconnect failed: WiFi not connected"));
    return false;
  }

  if (client.connected())
  {
    return true;
  }

  // Use the hostname from MDNSManager as the client ID
  char clientId[23];
  strlcpy(clientId, config.getSensorName(), sizeof(clientId)); // Use sensor name as client ID

  Serial.print("Attempting to connect to MQTT broker with client ID: ");
  Serial.println(clientId);

  return client.connect(clientId);
}

bool MQTTClient::publish(const String &message)
{
  if (!client.connected())
  {
    if (!reconnect()) // Attempt to reconnect if not connected
    {
      return false;
    }
  }
  Serial.print(F("Publishing MQTT message to topic: "));
  Serial.print(mqttTopic); // Use the updated MQTT topic
  Serial.print(F(". Message: "));
  Serial.println(message);

  if (client.publish(config.getMqttTopic(), message.c_str()))
  {
    return true;
  }
  else
  {
    Serial.println(F("Publish failed"));
    return false;
  }
}