#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include "Config.h"
#include "OTAUpdater.h"
#include "Sensor.h"
#include "WiFiManager.h"
#include "MDNSManager.h"

class WebServer {
public:
    WebServer(Config& config, OTAUpdater& otaUpdater, Sensor& sensor, WiFiManager& wifiManager, MDNSManager& mdnsManager);
    void begin();
    void handle();
    bool hasStarted() const; // Declaration of the hasStarted function

private:
    ESP8266WebServer server;
    Config& config;
    OTAUpdater& otaUpdater;
    Sensor& sensor;
    WiFiManager& wifiManager;
    MDNSManager& mdnsManager;
    bool serverStarted = false; // Track if the server has started

    void setupRoutes();
    void handleRoot();
    void handleConfigGet();
    void handleConfigPost();
    void handleOTAUpload();
    void handleWiFiScan();
    void handleWiFiConnect();
    void handleSensorData();
    void handleSensorStatus();
    void handleMDNSClients();
    void handleNotFound();
    void logRequest();
    String processTemplate(const String& templateContent);
    bool captivePortal();
    bool isIp(const String& str);
    String toStringIP(const IPAddress& ip);
};

#endif // WEB_SERVER_H
