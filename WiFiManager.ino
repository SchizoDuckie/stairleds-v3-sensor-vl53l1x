#include "WiFiManager.h" 
#include <ArduinoJson.h>

WiFiManager::WiFiManager(Config& cfg) 
  : apMode(false), 
    config(cfg),
    apIP(10, 0, 0, 1),
    apGateway(10, 0, 0, 1),
    apSubnet(255, 255, 255, 0) {}

void WiFiManager::setup() {
  WiFi.mode(WIFI_STA);
  String ssid = config.getWifiSSID();
  String password = config.getWifiPassword();
  Serial.print(F("Attempting to connect to ap from config: "));
  Serial.print(ssid);
  Serial.print(F(" Password: "));
  Serial.println(password);
  if (!connect(ssid, password)) {
    startAPMode();
  }
}

void WiFiManager::handle() {
  if (apMode) {
    MDNS.update();
  } else if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi connection lost. Attempting to reconnect..."));
    
    if (!connect(config.getWifiSSID(), config.getWifiPassword())) {
      startAPMode();
    }
  }
}

bool WiFiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::startAPMode() {
  apMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apGateway, apSubnet);
  String apName = "StairledSensor-" + String(ESP.getChipId());
  WiFi.softAP(apName.c_str());
  Serial.print(F("Started AP Mode. SSID: "));
  Serial.println(apName);
  Serial.print(F("AP IP address: "));
  Serial.println(WiFi.softAPIP());
  
  setupMDNS();
}

void WiFiManager::stopAPMode() {
  if (apMode) {
    apMode = false;
    WiFi.softAPdisconnect(true);
    MDNS.end();
    Serial.println(F("Stopped AP Mode"));
  }
}

void WiFiManager::setupMDNS() {
  if (MDNS.begin("sensor")) {
    MDNS.addService("http", "tcp", 80);
    Serial.println(F("mDNS responder started"));
    Serial.println(F("You can now connect to http://sensor.local"));
  } else {
    Serial.println(F("Error setting up mDNS responder!"));
  }
}

// Set a reasonable size for the JsonDocument to avoid dynamic memory allocation issues
String WiFiManager::scanNetworksJson() {
  const size_t capacity = JSON_ARRAY_SIZE(20) + JSON_OBJECT_SIZE(2) * 20; // Assuming a maximum of 20 networks
  StaticJsonDocument<capacity> doc; // Using a StaticJsonDocument for better memory management
  JsonArray jsonNetworks = doc.to<JsonArray>();

  scanNetworks(); // Make sure networks are populated before serialization

  for (const auto& network : networks) {
    JsonObject jsonNetwork = jsonNetworks.add<JsonObject>();
    jsonNetwork["ssid"] = network.ssid;
    jsonNetwork["rssi"] = network.rssi;
  }

  String result;
  if (serializeJson(doc, result) == 0) {
    Serial.println(F("Failed to serialize JSON!"));
    return "{}"; // Return an empty JSON object if serialization fails
  }
  return result;
}

bool WiFiManager::connect(const String& ssid, const String& password) {

  if (ssid.length() == 0) {
    Serial.print(F("WifiManager.connect was passed an empty ssid, wtf"));
    return false;
  }

  if (apMode) {
    Serial.print(F("Stopping AP mode: "));
    stopAPMode();
  }

  Serial.print(F("Attempting to connect to: "));
  Serial.println(ssid.c_str());
  Serial.print(F(" password: "));
  Serial.println(password.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\nConnected to WiFi"));
    return true;
  } else {
    Serial.println(F("\nFailed to connect to WiFi. Password incorrect?"));
    return false;
  }
}

void WiFiManager::scanNetworks() {
  networks.clear();
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    networks.push_back({WiFi.SSID(i), WiFi.RSSI(i)});
  }
}

bool WiFiManager::isInAPMode() const {
  return apMode;
}

IPAddress WiFiManager::getAPIP() const {
  return apIP;
}
