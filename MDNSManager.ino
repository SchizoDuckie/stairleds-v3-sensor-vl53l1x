#include "MDNSManager.h"

MDNSManager::MDNSManager(Config& config) : config(config) {
    createHostname();
}

bool MDNSManager::begin() {
    if (MDNS.begin(hostname.c_str())) {
        MDNS.addService("http", "tcp", 80);
        Serial.print(F("mDNS responder started: http://"));
        Serial.print(hostname);
        Serial.println(F(".local"));
        return true;
    } else {
        Serial.println(F("Error setting up mDNS responder!"));
        return false;
    }
}

void MDNSManager::update() {
    MDNS.update();
}

void MDNSManager::end() {
    MDNS.end();
}

String MDNSManager::getHostname() const {
    return hostname + ".local";
}

bool MDNSManager::discoverServer(String& serverIP, uint16_t& serverPort) {
    int n = MDNS.queryService("http", "tcp");
    for (int i = 0; i < n; ++i) {
        if (MDNS.hostname(i) == "stairled-server") {
            serverIP = MDNS.IP(i).toString();
            serverPort = MDNS.port(i);
            return true;
        }
    }
    return false;
}

void MDNSManager::createHostname() {
    hostname = "stairled-sensor-";
    if (strlen(config.getSensorName()) > 0) {
        hostname += sanitizeName(config.getSensorName());
    } else {
        hostname += String(ESP.getChipId(), HEX);
    }
    
    if (hostname.length() > 63) {
        hostname = hostname.substring(0, 63);
    }
}

String MDNSManager::sanitizeName(const String& name) {
    String sanitized = name;
    sanitized.toLowerCase();
    sanitized.replace(" ", "-");
    
    // Remove any character that's not a-z, 0-9, or -
    for (int i = sanitized.length() - 1; i >= 0; i--) {
        char c = sanitized.charAt(i);
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) {
            sanitized.remove(i, 1);
        }
    }
    
    return sanitized;
}
