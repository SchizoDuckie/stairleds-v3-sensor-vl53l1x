#ifndef MDNS_MANAGER_H
#define MDNS_MANAGER_H

#include <ESP8266mDNS.h>
#include "Config.h"


class MDNSManager {
public:
    MDNSManager(Config& config);
    bool begin();
    void handle();
    void end();
    String getHostname() const;
    static bool mDNSStarted;
    bool discoverServer(String& serverIP, uint16_t& serverPort);

private:
    Config& config;
    String hostname;
    void createHostname(const char *sensorName);
};

#endif // MDNS_MANAGER_H
