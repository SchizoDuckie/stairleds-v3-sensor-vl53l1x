#include "MDNSManager.h"

MDNSManager::MDNSManager(Config& config) : config(config) {
    }

bool MDNSManager::mDNSStarted = false;

bool MDNSManager::begin() {
    if (!mDNSStarted) {
        createHostname(config.getSensorName());
        if (MDNS.begin(hostname.c_str())) {
            MDNS.addService("http", "tcp", 80);
            MDNS.addServiceTxt("http", "tcp", "path", "/");
            MDNS.addServiceTxt("http", "tcp", "version", "1.0");
            Serial.print(F("mDNS responder started: http://"));
            Serial.print(getHostname());
            mDNSStarted = true;
            return true;
        } else {
            Serial.println(F("Failed to start mDNS responder!"));
            return false;
        }
    } else {
        Serial.println(F("mDNS responder already started."));
        return true;
    }
}

unsigned long lastAnnouncementTime = 0; // New member variable

void MDNSManager::handle() {
    if (mDNSStarted)
    {
        MDNS.update();
    }
}

void MDNSManager::end() {
    MDNS.end();
    mDNSStarted = false;
}

String MDNSManager::getHostname() const {
    return hostname + ".local";
}

bool MDNSManager::discoverServer(String& serverIP, uint16_t& serverPort) {
    int n = MDNS.queryService("http", "tcp");
    for (int i = 0; i < n; ++i) {
        if (MDNS.hostname(i) == "stairled-server.local") {
            serverIP = MDNS.IP(i).toString();
            serverPort = MDNS.port(i);
            return true;
        }
    }
    return false;
}

void MDNSManager::createHostname(const char *sensorName)
{
    hostname = "stairled-sensor-";
    if (strlen(sensorName) > 0) {
        hostname += sensorName; 
    } else {
        hostname += String(ESP.getChipId(), HEX);
    }
    
    if (hostname.length() > 63) {
        hostname = hostname.substring(0, 63);
    }
}



