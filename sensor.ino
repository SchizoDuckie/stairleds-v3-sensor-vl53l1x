// new-sensor.ino (Main project file)

#include "Config.h"
#include "Sensor.h"
#include "WiFiManager.h"
#include "OTAUpdater.h"
#include "WebConfig.h"
#include "MQTTClient.h"

Config config;
Sensor sensor;
WiFiManager wifiManager;
OTAUpdater otaUpdater;
WebConfig webConfig;
MQTTClient mqttClient;

void setup() {
  Serial.begin(115200);
  
  config.load();
  wifiManager.setup();
  sensor.setup();
  otaUpdater.setup();
  webConfig.setup();
  mqttClient.setup();
}

void loop() {
  wifiManager.handle();
  webConfig.handle();
  
  if (wifiManager.isConnected()) {
    if (!mqttClient.isConnected()) {
      mqttClient.reconnect();
    }
    mqttClient.handle();
    
    sensor.read();
    mqttClient.publish(sensor.getData());
  }
  
  // Implement watchdog timer reset
  ESP.wdtFeed();
}
