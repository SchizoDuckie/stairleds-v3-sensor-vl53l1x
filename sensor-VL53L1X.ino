#include "Config.h"
#include "Sensor.h"
#include "WiFiManager.h"
#include "OTAUpdater.h"
#include "WebServer.h"
#include "MQTTClient.h"

Config config;
Sensor sensorManager;
WiFiManager wifiManager(config);
OTAUpdater otaUpdater;
WebServer webServer(config, otaUpdater, sensorManager, wifiManager);
MQTTClient mqttClient(config);

void setup() {
  Serial.begin(115200);
  Serial.println(F("\nStairled Sensor Starting..."));
  
  config.load();
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
  ESP.wdtFeed();
}