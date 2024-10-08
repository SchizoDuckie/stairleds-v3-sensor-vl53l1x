// OTAUpdater.h

#ifndef OTA_UPDATER_H
#define OTA_UPDATER_H

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

class OTAUpdater {
public:
  void setup();
  void handle();
  void performUpdate(const String& url);
};

#endif
