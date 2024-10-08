// WebConfig.h

#ifndef WEB_CONFIG_H
#define WEB_CONFIG_H

#include <ESP8266WebServer.h>
#include <Base64.h> // Include Base64 library

class WebConfig {
public:
  void setup();
  void handle();

private:
  ESP8266WebServer server;
  void handleRoot();
  void handleWiFiUpdate();
  void handleOTAUpdate();

  void encodeCSS();
};

#endif
