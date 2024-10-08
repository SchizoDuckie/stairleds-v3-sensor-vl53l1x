// OTAUpdater.cpp

#include "OTAUpdater.h"
#include "Config.h"

void OTAUpdater::setup() {
  // Setup OTA, if needed
}

void OTAUpdater::handle() {
  // Check for updates periodically
}

void OTAUpdater::performUpdate(const String& url) {
  WiFiClient client;
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);
  
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}
