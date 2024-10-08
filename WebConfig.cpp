// WebConfig.cpp

#include "WebConfig.h"
#include "Config.h"
#include "OTAUpdater.h"

extern OTAUpdater otaUpdater;

// Minimal CSS framework embedded in PROGMEM
const char cssFramework[] PROGMEM = R"rawliteral(
body {
  font-family: Arial, sans-serif;
  background-color: #f4f4f4;
  color: #333;
  margin: 0;
  padding: 0;
}

.container {
  width: 80%;
  max-width: 800px;
  margin: 20px auto;
  padding: 20px;
  background-color: #fff;
  border-radius: 8px;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
}

h1 {
  color: #4CAF50;
  text-align: center;
}

form {
  margin-bottom: 20px;
}

input[type="text"],
input[type="password"] {
  width: 100%;
  padding: 10px;
  margin: 8px 0;
  border: 1px solid #ccc;
  border-radius: 4px;
}

input[type="submit"] {
  background-color: #4CAF50;
  color: white;
  padding: 14px 20px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  width: 100%;
}

input[type="submit"]:hover {
  background-color: #45a049;
}

.alert {
  color: #d9534f;
}
)rawliteral";

// HTML template for the web interface
const char htmlTemplate[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Stairled Sensor Configuration</title>
  <style>
    @import url('data:text/css;base64,{0}');
  </style>
</head>
<body>
  <div class="container">
    <h1>Stairled Sensor Configuration</h1>
    <form action='/api/wifi' method='post'>
      <label for="ssid">SSID:</label>
      <input type="text" name="ssid" required>
      <label for="password">Password:</label>
      <input type="password" name="password" required>
      <input type="submit" value="Update WiFi">
    </form>
    <form action='/api/update' method='post'>
      <label for="otaUrl">OTA URL:</label>
      <input type="text" name="otaUrl" required>
      <input type="submit" value="Update Firmware">
    </form>
  </div>
</body>
</html>
)rawliteral";

void WebConfig::setup() {
  // Encode CSS and replace the placeholder in the HTML template
  String base64CSS = base64::encode(cssFramework);
  String updatedHtmlTemplate;
  updatedHtmlTemplate.reserve(strlen(htmlTemplate) + base64CSS.length() + 1); // Reserve enough space
  updatedHtmlTemplate = htmlTemplate;
  updatedHtmlTemplate.replace("{0}", base64CSS);

  server.on("/", HTTP_GET, [this, updatedHtmlTemplate]() {
    server.send(200, "text/html", updatedHtmlTemplate);
  });
  
  server.on("/api/wifi", HTTP_POST, [this]() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    config.wifiSSID = ssid;
    config.wifiPassword = password;
    config.save();
    server.send(200, "text/plain", "WiFi settings updated. Rebooting...");
    delay(1000);
    ESP.restart();
  });

  server.on("/api/update", HTTP_POST, [this]() {
    String otaUrl = server.arg("otaUrl");
    otaUpdater.performUpdate(otaUrl);
    server.send(200, "text/plain", "OTA Update initiated.");
  });

  server.begin();
}

void WebConfig::handle() {
  server.handleClient();
}
