#include "WebServer.h"
#include <ArduinoJson.h>

// Define string constants
const char* const HTTP_200_TEXT_PLAIN = "text/plain";
const char* const HTTP_200_TEXT_HTML = "text/html";
const char* const HTTP_400_TEXT_PLAIN = "400";
const char* const HTTP_404_TEXT_PLAIN = "404";
const char* const HTTP_APPLICATION_JSON = "application/json";
const char* const WIFI_SCAN_ENDPOINT = "/api/wifi-scan";
const char* const WIFI_CONNECT_ENDPOINT = "/api/wifi-connect";
const char* const CONFIG_ENDPOINT = "/api/config";
const char* const SENSOR_DATA_ENDPOINT = "/api/sensor-data";
const char* const SENSOR_STATUS_ENDPOINT = "/api/sensor-status";
const char* const MDNS_CLIENTS_ENDPOINT = "/api/mdns-clients";
const char* const OTA_ENDPOINT = "/api/ota";
const char* const ROOT_URI = "/";
const char* const CAPTIVE_PORTAL_REDIRECT = "http://";
const char* const FILE_NOT_FOUND_MSG = "File Not Found\n\n";
const char* const JSON_INVALID_MSG = "Invalid JSON";
const char* const CONFIG_UPDATED_MSG = "Configuration updated successfully";
const char* const CONNECTION_FAILED_MSG = "Connection failed";
const char* const NO_DATA_RECEIVED_MSG = "No data received";
const char* const HEADER_LOCATION = "Location";

WebServer::WebServer(Config& config, OTAUpdater& otaUpdater, Sensor& sensor, WiFiManager& wifiManager, MDNSManager& mdnsManager)
    : server(80), config(config), otaUpdater(otaUpdater), sensor(sensor), wifiManager(wifiManager), mdnsManager(mdnsManager) {}

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
    server.on(SENSOR_DATA_ENDPOINT, HTTP_GET, std::bind(&WebServer::handleSensorData, this));
    server.on(SENSOR_STATUS_ENDPOINT, HTTP_GET, std::bind(&WebServer::handleSensorStatus, this));
    server.on(MDNS_CLIENTS_ENDPOINT, HTTP_GET, std::bind(&WebServer::handleMDNSClients, this));

    server.serveStatic(ROOT_URI, LittleFS, ROOT_URI);
    
    server.onNotFound(std::bind(&WebServer::handleNotFound, this));

    server.addHook([this](const String& method, const String& url, WiFiClient* client, ESP8266WebServer::ContentTypeFunction contentType) {
        (void)method;      // Cast to void to avoid unused variable warnings
        (void)url;
        (void)client;
        (void)contentType;
        this->logRequest();
        return ESP8266WebServer::CLIENT_REQUEST_CAN_CONTINUE;
    });
}


void WebServer::logRequest() {
    String message = server.method() == HTTP_GET ? F("GET") : F("POST");
    message += F(" ");
    message += server.uri();
    
    if (server.args() > 0) {
        message += F("\nArguments: ");
        for (uint8_t i = 0; i < server.args(); i++) {
            message += F("\n");
            message += server.argName(i);
            message += F(": ");
            message += server.arg(i);
        }
    }
    Serial.println(message);
}


void WebServer::handle() {
    server.handleClient();
}

void WebServer::handleSensorData() {
    server.send(200, HTTP_200_TEXT_PLAIN, String(sensor.getDistance()));
}

void WebServer::handleConfigGet() {
    JsonDocument doc;
    doc["sensorName"] = config.getSensorName();
    doc["mqttBroker"] = config.getMqttBroker();
    doc["mqttPort"] = config.getMqttPort();
    doc["mqttTopic"] = config.getMqttTopic();

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, HTTP_APPLICATION_JSON, jsonResponse);
}


void WebServer::handleConfigPost() {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));

    if (error) {
        server.send(400, HTTP_200_TEXT_PLAIN, JSON_INVALID_MSG);
        return;
    }

    if (doc["sensorName"].is<const char*>()) {
        config.setSensorName(doc["sensorName"]);
    }

    if (doc["wifiSSID"].is<const char*>() && doc["wifiPassword"].is<const char*>()) {
        config.setWifiCredentials(doc["wifiSSID"], doc["wifiPassword"]);
    }

    if (doc["mqttBroker"].is<const char*>()) {
        config.setMqttBroker(doc["mqttBroker"]);
    }

    if (doc["mqttPort"].is<uint16_t>()) {
        config.setMqttPort(doc["mqttPort"]);
    }

    if (doc["mqttTopic"].is<const char*>()) {
        config.setMqttTopic(doc["mqttTopic"]);
    }
    
    bool needsReboot = false;

    if (doc.containsKey("sensorName")) {
        String newName = doc["sensorName"].as<String>();
        if (strcmp(newName.c_str(), config.getSensorName()) != 0) {
            config.setSensorName(newName.c_str());
            needsReboot = true;
        }
    }

    if (doc.containsKey("wifiSSID") && doc.containsKey("wifiPassword")) {
        const char* newSSID = doc["wifiSSID"];
        const char* newPassword = doc["wifiPassword"];
        if (strcmp(newSSID, config.getWifiSSID()) != 0 || 
            strcmp(newPassword, config.getWifiPassword()) != 0) {
            config.setWifiCredentials(newSSID, newPassword);
            needsReboot = true;
        }
    }

    if (doc.containsKey("mqttBroker")) {
        const char* newBroker = doc["mqttBroker"];
        if (strcmp(newBroker, config.getMqttBroker()) != 0) {
            config.setMqttBroker(newBroker);
            needsReboot = true;
        }
    }

    if (doc.containsKey("mqttPort")) {
        uint16_t newPort = doc["mqttPort"];
        if (newPort != config.getMqttPort()) {
            config.setMqttPort(newPort);
            needsReboot = true;
        }
    }

    if (doc.containsKey("mqttTopic")) {
        const char* newTopic = doc["mqttTopic"];
        if (strcmp(newTopic, config.getMqttTopic()) != 0) {
            config.setMqttTopic(newTopic);
            needsReboot = true;
        }
    }

    config.save();

    JsonObject response = doc.to<JsonObject>();
    response["success"] = true;
    response["needsReboot"] = needsReboot;

    String jsonResponse;
    serializeJson(response, jsonResponse);
    server.send(200, HTTP_APPLICATION_JSON, jsonResponse);

    if (needsReboot) {
        delay(1000); // Give time for the response to be sent
        ESP.restart();
    }
}

void WebServer::handleSensorStatus() {
    JsonDocument doc;
    
    doc["sensorName"] = config.getSensorName();
    
    if (WiFi.status() == WL_CONNECTED) {
        doc["connected"] = true;
        doc["ssid"] = WiFi.SSID();
        doc["ip"] = WiFi.localIP().toString();
        doc["isAPMode"] = false;
        doc["mdnsHostname"] = mdnsManager.getHostname();

        String serverIP;
        uint16_t serverPort;
        if (mdnsManager.discoverServer(serverIP, serverPort)) {
            doc["serverDiscovered"] = true;
            doc["serverIP"] = serverIP;
            doc["serverPort"] = serverPort;
        } else {
            doc["serverDiscovered"] = false;
        }
    } else if (wifiManager.isInAPMode()) {
        doc["connected"] = false;
        doc["isAPMode"] = true;
        doc["apName"] = WiFi.softAPSSID();
        doc["apIP"] = WiFi.softAPIP().toString();
    } else {
        doc["connected"] = false;
        doc["isAPMode"] = false;
    }

    String response;
    serializeJson(doc, response);
    
    Serial.println("Sensor status response:");
    Serial.println(response);
    
    server.send(200, HTTP_APPLICATION_JSON, response);
}

void WebServer::handleRoot() {
    if (captivePortal()) { return; }
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
        server.send(404, HTTP_200_TEXT_PLAIN, FILE_NOT_FOUND_MSG);
        return;
    }

    server.streamFile(file, HTTP_200_TEXT_HTML);
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
        
        config.setWifiCredentials(ssid.c_str(), password.c_str());
        config.save();
        
        // Send response before resetting
        server.send(200, HTTP_200_TEXT_PLAIN, "WiFi credentials saved. Device will reset and connect to the new network.");
        
        // Delay to ensure the response is sent
        delay(1000);
        
        // Reset the device
        ESP.restart();
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
    if (!isIp(server.hostHeader()) && server.hostHeader() != (String(config.getSensorName()) + ".local")) {
        Serial.println(F("Request redirected to captive portal"));
        server.sendHeader(HEADER_LOCATION, String(CAPTIVE_PORTAL_REDIRECT) + toStringIP(server.client().localIP()), true);
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

void WebServer::handleMDNSClients() {
    if (WiFi.status() != WL_CONNECTED) {
        // Device is not connected to WiFi
        JsonDocument errorDoc;
        errorDoc["error"] = "Not connected to WiFi";
        errorDoc["isAPMode"] = wifiManager.isInAPMode();
        errorDoc["connected"] = false;

        String errorResponse;
        serializeJson(errorDoc, errorResponse);
        server.send(400, HTTP_APPLICATION_JSON, errorResponse);
        return;
    }

    // Create the proper mDNS name
    String mdnsName = String(config.getSensorName()) + ".local";

    // Start mDNS with the correct name
    if (!MDNS.begin(mdnsName.c_str())) {
        JsonDocument errorDoc;
        errorDoc["error"] = "Failed to start mDNS";

        String errorResponse;
        serializeJson(errorDoc, errorResponse);
        server.send(500, HTTP_APPLICATION_JSON, errorResponse);
        return;
    }

    int n = MDNS.queryService("http", "tcp");  // Query HTTP services on the network
    JsonDocument responseDoc;
    JsonArray clientsArray = responseDoc["clients"].to<JsonArray>();

    for (int i = 0; i < n; ++i) {
        JsonObject client = clientsArray.add<JsonObject>();
        client["hostname"] = MDNS.hostname(i);
        client["ip"] = MDNS.IP(i).toString();
        client["port"] = MDNS.port(i);
    }

    MDNS.end();  // Stop mDNS

    // Serialize response and send it
    String jsonResponse;
    serializeJson(responseDoc, jsonResponse);
    server.send(200, HTTP_APPLICATION_JSON, jsonResponse);
}


String WebServer::toStringIP(const IPAddress& ip) {
    return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}