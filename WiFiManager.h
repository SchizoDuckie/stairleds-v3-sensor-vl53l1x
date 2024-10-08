// WiFiManager.h

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <DNSServer.h>

class WiFiManager {
public:
  void setup();
  void handle();
  bool isConnected();
  void startAPMode();
  void scanNetworks();

private:
  DNSServer dnsServer;
  bool apMode;
};

#endif
