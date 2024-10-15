#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include "MDNSManager.h"
#include <vector>

struct WiFiNetwork {
  String ssid;
  int32_t rssi;
};

class WiFiManager {
public:
  WiFiManager(Config& cfg, MDNSManager& mdnsMgr);
  void setup();
  void handle();
  bool isConnected() const;
  void startAPMode();
  void stopAPMode();
  String scanNetworksJson();
  bool connect(const String& ssid, const String& password);
  bool isInAPMode() const;
  IPAddress getAPIP() const;

private:
  Config& config;
  MDNSManager& mdnsManager;
  std::vector<WiFiNetwork> networks;
  void scanNetworks();
  bool apMode;
  IPAddress apIP;
  IPAddress apGateway;
  IPAddress apSubnet;
  void setupMDNS();
};

#endif // WIFI_MANAGER_H
