#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include "Config.h"
#include "OTAUpdater.h"
#include "Sensor.h"
#include "WiFiManager.h"

class WebServer {
public:
    WebServer(Config& config, OTAUpdater& otaUpdater, Sensor& sensor, WiFiManager& wifiManager);
    void begin();
    void handle();

private:
    ESP8266WebServer server;
    Config& config;
    OTAUpdater& otaUpdater;
    Sensor& sensor;
    WiFiManager& wifiManager;

    void setupRoutes();
    void handleRoot();
    void handleConfigGet();
    void handleConfigPost();
    void handleOTAUpload();
    void handleWiFiScan();
    void handleWiFiConnect();
    void handleSensorData();
    void handleSensorStatus();
    void handleNotFound();
    void logRequest();
    String processTemplate(const String& templateContent);
    bool captivePortal();
    bool isIp(const String& str);
    String toStringIP(const IPAddress& ip);
};

#endif // WEB_SERVER_H