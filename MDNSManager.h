#ifndef MDNS_MANAGER_H
#define MDNS_MANAGER_H

#include <ESP8266mDNS.h>
#include "Config.h"

class MDNSManager {
public:
    MDNSManager(Config& config);
    bool begin();
    void update();
    void end();
    String getHostname() const;
    bool discoverServer(String& serverIP, uint16_t& serverPort);
    static String sanitizeName(const String& name);

private:
    Config& config;
    String hostname;
    void createHostname();
};

#endif // MDNS_MANAGER_H
