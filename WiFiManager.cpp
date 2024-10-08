// WiFiManager.cpp

#include "WiFiManager.h"
#include "Config.h"

void WiFiManager::setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.wifiSSID.c_str(), config.wifiPassword.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    apMode = false;
  } else {
    Serial.println("\nFailed to connect. Starting AP mode.");
    startAPMode();
  }
}

void WiFiManager::handle() {
  if (apMode) {
    dnsServer.processNextRequest();
  }
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::startAPMode() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(("StairLedSensor-" + String(ESP.getChipId())).c_str());
  dnsServer.start(53, "*", WiFi.softAPIP());
  apMode = true;
}

void WiFiManager::scanNetworks() {
  Serial.println("Scanning for networks...");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    if (WiFi.SSID(i).startsWith("stairled-")) {
      Serial.println(WiFi.SSID(i));
    }
  }
}
