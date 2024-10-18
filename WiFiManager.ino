#include "WiFiManager.h" 
#include <ArduinoJson.h>

WiFiManager::WiFiManager(Config& cfg, MDNSManager& mdnsMgr) 
  : apMode(false), 
    config(cfg),
    mdnsManager(mdnsMgr),
    apIP(10, 0, 0, 1),
    apGateway(10, 0, 0, 1),
    apSubnet(255, 255, 255, 0) {}

void WiFiManager::setup() {
  Serial.print(F("WifiManager::Setup"));
  WiFi.mode(WIFI_STA);
 }

void WiFiManager::handle() {
  // Check if we are in AP mode and periodically attempt to reconnect
  static unsigned long lastReconnectAttempt = 0; // Track the last reconnect attempt time
  const unsigned long reconnectInterval = 10000; // Attempt to reconnect every 10 seconds

  if (WiFi.status() != WL_CONNECTED && !apMode) {
    Serial.println(F("WiFi connection lost. Attempting to reconnect..."));
    
    String ssid = config.getWifiSSID();
    String password = config.getWifiPassword();

    if (ssid.length() > 0) {
      if (!connect(ssid, password)) {
        Serial.println(F("Reconnection failed. Starting AP mode."));
        startAPMode();
      }
    } else {
      Serial.println(F("No configured WiFi found. Starting AP mode."));
      startAPMode();
    }
  } else if (apMode) {
    // If in AP mode, periodically attempt to reconnect
    if (millis() - lastReconnectAttempt >= reconnectInterval) {
      lastReconnectAttempt = millis(); // Update the last reconnect attempt time
      String ssid = config.getWifiSSID();
      String password = config.getWifiPassword();

      if (ssid.length() > 0) {
        Serial.println(F("Attempting to reconnect to original AP..."));
        connect(ssid, password); // Attempt to reconnect
      }
    }
  }
}

bool WiFiManager::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::startAPMode() {
  Serial.println(F("WifiManager::startAPMode."));
  if (apMode)
  {
    Serial.println(F("Already in AP mode. Skipping AP start."));
    return;
    }
    
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apGateway, apSubnet);
    
    // Use the sensor name in the AP name
    String apName = "stairled-sensor-";
    if (strlen(config.getSensorName()) > 0) {
        apName += config.getSensorName();
    } else {
        apName += String(ESP.getChipId(), HEX); // Fallback to chip ID if no name set
    }
    
    // Ensure the AP name is not too long (max 32 characters for ESP8266)
    if (apName.length() > 32) {
        apName = apName.substring(0, 32);
    }
    
    WiFi.softAP(apName.c_str());
    
    Serial.print(F("Started AP Mode. SSID: "));
    Serial.println(apName);
    Serial.print(F("AP IP address: "));
    Serial.println(WiFi.softAPIP());
    apMode = true;
    setupMDNS();
}

void WiFiManager::stopAPMode() {
  Serial.println(F("WifiManager::stopAPMode."));
  if (apMode) {
    apMode = false;
    WiFi.softAPdisconnect(true);
    MDNS.end();
    Serial.println(F("Stopped AP Mode"));
  }
}

void WiFiManager::setupMDNS() {
  Serial.println(F("WifiManager::setupMDNS."));
  mdnsManager.begin();
}

// Set a reasonable size for the JsonDocument to avoid dynamic memory allocation issues
String WiFiManager::scanNetworksJson() {
  Serial.println(F("Scanning for nearby networks to serialize to json"));
  JsonDocument doc;
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
  Serial.println(F("Wifimanager::connect"));

  if (ssid.length() == 0) {
    Serial.println(F("WifiManager.connect was passed an empty ssid, aborting connect."));
    return false;
  }

  if (apMode) {
    Serial.println(F("Stopping AP mode"));
    stopAPMode();
  }

  Serial.print(F("Attempting to connect to: "));
  Serial.println(ssid);
  Serial.print(F("Password: "));
  Serial.println(password);
  WiFi.begin(ssid.c_str(), password.c_str());

  int attempts = 0;
  unsigned long startAttemptTime = millis(); // Start time for connection attempt
  const unsigned long timeout = 30000; // 30 seconds timeout
  const unsigned long backoffInterval = 2000; // 5 seconds backoff

  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(200);
    Serial.print(".");
    attempts++;

    // Check if the timeout has been reached
    if (millis() - startAttemptTime >= timeout) {
      Serial.println(F("\nConnection attempt timed out. Starting AP mode."));
      startAPMode(); // Start AP mode if timeout occurs
      return false;
    }

    // Implement backoff strategy
    if (attempts > 5) {
      Serial.print(F("\nWifi not connected within "));
      Serial.print(attempts);
      Serial.println(F("attempts. Waiting 2s before next attempt..."));
      delay(backoffInterval);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\nConnected to WiFi"));
    
    Serial.println(F("Starting MDNS because WiFi is connected"));
    setupMDNS();  // Set up mDNS after successful connection
    return true;
  } else {
    Serial.println(F("\nFailed to connect to WiFi"));
    return false;
  }
}

void WiFiManager::scanNetworks() {
  Serial.print(F("WiFiManager::scanNetworks"));
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
