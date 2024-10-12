#include "WebServer.h"
#include <ArduinoJson.h>

// Define string constants
const char* const HTTP_200_TEXT_PLAIN = "text/plain";
const char* const HTTP_400_TEXT_PLAIN = "400";
const char* const HTTP_404_TEXT_PLAIN = "404";
const char* const HTTP_APPLICATION_JSON = "application/json";
const char* const WIFI_SCAN_ENDPOINT = "/api/wifi-scan";
const char* const WIFI_CONNECT_ENDPOINT = "/api/wifi-connect";
const char* const CONFIG_ENDPOINT = "/api/config";
const char* const OTA_ENDPOINT = "/api/ota";
const char* const ROOT_URI = "/";
const char* const CAPTIVE_PORTAL_REDIRECT = "http://";
const char* const FILE_NOT_FOUND_MSG = "File Not Found\n\n";
const char* const JSON_INVALID_MSG = "Invalid JSON";
const char* const CONFIG_UPDATED_MSG = "Configuration updated successfully";
const char* const CONNECTION_FAILED_MSG = "Connection failed";
const char* const NO_DATA_RECEIVED_MSG = "No data received";

WebServer::WebServer(Config& config, OTAUpdater& otaUpdater, Sensor& sensor, WiFiManager& wifiManager)
    : server(80), config(config), otaUpdater(otaUpdater), sensor(sensor), wifiManager(wifiManager) {}

void WebServer::begin() {
    if (!LittleFS.begin()) {
        Serial.println(F("An error has occurred while mounting LittleFS"));
        return;
    }

    setupRoutes();
    server.begin();
    Serial.println(F("WebServer started"));
}

void WebServer::setupRoutes() {
    server.on(ROOT_URI, HTTP_GET, std::bind(&WebServer::handleRoot, this));
    server.on(CONFIG_ENDPOINT, HTTP_GET, std::bind(&WebServer::handleConfigGet, this));
    server.on(CONFIG_ENDPOINT, HTTP_POST, std::bind(&WebServer::handleConfigPost, this));
    server.on(OTA_ENDPOINT, HTTP_POST, [this]() {
        server.sendHeader("Connection", "close");
        server.send(200, HTTP_200_TEXT_PLAIN, (Update.hasError()) ? "Update failed." : "Update success.");
        ESP.restart();
    }, std::bind(&WebServer::handleOTAUpload, this));
    server.on(WIFI_SCAN_ENDPOINT, HTTP_GET, std::bind(&WebServer::handleWiFiScan, this));
    server.on(WIFI_CONNECT_ENDPOINT, HTTP_POST, std::bind(&WebServer::handleWiFiConnect, this));
    server.on("/api/sensor-data", HTTP_GET, std::bind(&WebServer::handleSensorData, this));
    
    server.serveStatic("/", LittleFS, "/");
    
    server.onNotFound(std::bind(&WebServer::handleNotFound, this));
}

void WebServer::handle() {
    server.handleClient();
}

void WebServer::handleSensorData() {
    server.send(200, HTTP_200_TEXT_PLAIN, String(sensor.getDistance()));
}

void WebServer::handleConfigGet() {
    JsonDocument doc;
    doc["sensorName"] = config.sensorName;
    doc["mqttBroker"] = config.mqttBroker;
    doc["mqttPort"] = config.mqttPort;
    doc["mqttTopic"] = config.mqttTopic;

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, HTTP_APPLICATION_JSON, jsonResponse);
}

void WebServer::handleConfigPost() {
    if (server.hasArg("plain")) {
        String jsonBody = server.arg("plain");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonBody);

        if (error) {
            server.send(400, HTTP_200_TEXT_PLAIN, JSON_INVALID_MSG);
            return;
        }

        // Update config object
        strncpy(config.sensorName, doc["sensorName"] | config.sensorName, sizeof(config.sensorName) - 1);
        strncpy(config.mqttBroker, doc["mqttBroker"] | config.mqttBroker, sizeof(config.mqttBroker) - 1);
        strncpy(config.mqttTopic, doc["mqttTopic"] | config.mqttTopic, sizeof(config.mqttTopic) - 1);
        config.mqttPort = doc["mqttPort"] | config.mqttPort;

        // Save config
        config.save();

        server.send(200, HTTP_200_TEXT_PLAIN, CONFIG_UPDATED_MSG);
    } else {
        server.send(400, HTTP_200_TEXT_PLAIN, NO_DATA_RECEIVED_MSG);
    }
}

void WebServer::handleRoot() {
    if (captivePortal()) { return; }
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
        server.send(404, "text/plain", "File not found");
        return;
    }

    server.streamFile(file, "text/html");
    file.close();
}

void WebServer::handleOTAUpload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
    }
    yield();
}

void WebServer::handleWiFiScan() {
    String json = wifiManager.scanNetworksJson();
    server.send(200, HTTP_APPLICATION_JSON, json);
}

void WebServer::handleWiFiConnect() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        server.client().stop();
        server.close();
        //if (wifiManager.connect(ssid, password)) {
          strncpy(config.wifiSSID, ssid.c_str(), sizeof(config.wifiSSID) - 1);
          strncpy(config.wifiPassword, password.c_str(), sizeof(config.wifiPassword) - 1);
          config.save();
          Serial.printf("Save Success! Rebooting!: %s : %s! %s \n", ssid.c_str(), password.c_str());
          ESP.reset();
      
    } else {
        server.send(400, HTTP_200_TEXT_PLAIN, "Missing SSID or password");
    }
}

void WebServer::handleNotFound() {
    if (captivePortal()) { return; }
    String message = FILE_NOT_FOUND_MSG;
    message += F("URI: ");
    message += server.uri();
    message += F("\nMethod: ");
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += F("\nArguments: ");
    message += server.args();
    message += F("\n");
    for (uint8_t i = 0; i < server.args(); i++) {
        message += F(" ") + server.argName(i) + F(": ") + server.arg(i) + F("\n");
    }
    server.send(404, HTTP_404_TEXT_PLAIN, message);
    message = "";
}

bool WebServer::captivePortal() {
    if (!isIp(server.hostHeader()) && server.hostHeader() != (String(config.sensorName) + ".local")) {
        Serial.println(F("Request redirected to captive portal"));
        server.sendHeader("Location", String(CAPTIVE_PORTAL_REDIRECT) + toStringIP(server.client().localIP()), true);
        server.send(302, HTTP_200_TEXT_PLAIN, "");
        server.client().stop();
        return true;
    }
    return false;
}

bool WebServer::isIp(const String& str) {
    for (int i = 0; i < str.length(); i++) {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9')) {
            return false;
        }
    }
    return true;
}

String WebServer::toStringIP(const IPAddress& ip) {
    return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}
