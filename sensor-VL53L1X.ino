#include "Config.h"
#include "Sensor.h"
#include "WiFiManager.h"
#include "OTAUpdater.h"
#include "WebServer.h"
#include "MQTTClient.h"
#include "MDNSManager.h"

Config config;
Sensor sensorManager;
MDNSManager mdnsManager(config);
WiFiManager wifiManager(config, mdnsManager);
OTAUpdater otaUpdater;
WebServer webServer(config, otaUpdater, sensorManager, wifiManager, mdnsManager);
MQTTClient mqttClient(config);

void setup() {
  Serial.begin(115200);
  
  
  config.load();
  Serial.print(F("\nStairled Sensor Starting..."));
  Serial.print(F("\nSensor Name: "));
  Serial.print(config.getSensorName());
  Serial.print(F("\nMQTT Broker: "));
  Serial.print(config.getMqttBroker());
  Serial.print(F("\nMQTT Port: "));
  Serial.print(config.getMqttPort());
  Serial.print(F("\nMQTT Topic: "));
  Serial.print(config.getMqttTopic());
  Serial.print(F("\nWiFi SSID: "));
  Serial.print(config.getWifiSSID());
  Serial.print(F("\nWiFi Password: "));
  Serial.print(config.getWifiPassword());
 
  sensorManager.setup();
  wifiManager.setup();
  otaUpdater.setup();
  webServer.begin();
  //mqttClient.setup();
}

void loop() {
  wifiManager.handle();
  webServer.handle();
  otaUpdater.handle();
  //mqttClient.handle();

  static unsigned long lastSensorRead = 0;
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastSensorRead >= 100) { // Read sensor every 100ms
    if (sensorManager.read()) {
            
      lastSensorRead = currentMillis;
      //Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

      if (wifiManager.isConnected() && mqttClient.isConnected()) {
        static unsigned long lastMqttPublish = 0;
        if (currentMillis - lastMqttPublish >= 5000) { // Publish to MQTT every 5 seconds
          //mqttClient.publish(sensorManager.getData());
          lastMqttPublish = currentMillis;
        }
      }
    }
  }
    
  
  
  // Implement watchdog timer reset
  yield();
}
