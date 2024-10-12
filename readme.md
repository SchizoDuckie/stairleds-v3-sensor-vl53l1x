# Stairled Sensor Design Document

## 1. Sensor Overview

The Stairled Sensor is an ESP8266-based device that uses a VL53L1X Time-of-Flight sensor to detect movement on stairs. It connects to a dedicated WiFi network and publishes sensor data via MQTT.

## 2. Hardware Components

- ESP8266 microcontroller
- VL53L1X Time-of-Flight sensor
- 5V power supply from the server

## 3. Functionality Requirements

### 3.1 WiFi Connectivity
- Scan for available "stairled-*" networks
- Connect to configured "stairled-*" network
- Create its own AP with 10.0.0.1 ip address with a unique name (e.g., "stairled-sensor-<uniqid>") if no configured network is found
- Periodically check for previously configured network when in AP mode

### 3.2 Sensor Operation
- Continuously measure distance using VL53L1X sensor
- Publish raw sensor data to MQTT broker

### 3.3 Configuration
- Provide a modern, user-friendly captive portal for initial setup and reconfiguration
- Store configuration in non-volatile memory (e.g., EEPROM or SPIFFS)
- Offer API endpoint for remote WiFi parameter updates
- Store Web assets in LittleFS filesystem. 
  Use arduino-littlefs-upload-1.2.0.vsix for Arduino IDE to sync data:
  https://github.com/earlephilhower/arduino-littlefs-upload/releases/tag/1.2.0

### 3.4 MQTT Communication
- Connect to configured MQTT broker
- Publish sensor data on specified topics

### 3.5 OTA Updates
- Support Over-The-Air firmware updates by uploading a binary in the web interface

## 4. Configuration Options

- Sensor Name/ID
- WiFi credentials for "stairled-*" network
- MQTT Broker address
- MQTT Topics for publishing data
- GitHub URL for firmware updates

## 5. User Interface

### 5.1 Captive Portal Interface
- Modern, responsive design
- WiFi network selection (scan for "stairled-*" networks)
- Sensor name configuration
- MQTT settings configuration
- Real-time sensor measurement graph for debugging
- Firmware update interface with GitHub URL input

### 5.2 API Endpoints
- `/api/wifi` - POST endpoint to update WiFi parameters remotely
- `/api/update` - POST endpoint to trigger firmware update from provided GitHub URL

## 6. MQTT Topic Structure

- Sensor data: `stairled/<sensor-id>/data`
- Sensor status: `stairled/<sensor-id>/status`

## 7. Failsafe and Recovery

- Implement watchdog timer to recover from crashes
- Fallback to AP mode if unable to connect to configured network
- Store last known good configuration
- Periodically attempt to reconnect to last known good configuration when in AP mode

## 8. Testing and Validation

- Distance measurement accuracy tests
- WiFi connection stability tests
- MQTT communication reliability tests
- Long-term stability testing
- OTA update process testing

## 9. OTA Update Process

### 9.1 Generating OTA Update

1. Compile the firmware in Arduino IDE
2. Export the compiled binary: `Sketch` > `Export Compiled Binary`
3. Upload the `.bin` file to a GitHub release
4. Trigger update via API endpoint or captive portal interface

### 9.3 Update Process

1. User provides upload blob in fileupload on sensor interface
3. Sensor verifies the upload and applies the update
4. Sensor reboots with new firmware

## 10. Documentation Needs

- User manual for setup and operation
- API documentation for MQTT topic structure and payload formats
- Troubleshooting guide
- OTA update process guide for end-users, including how to create and use GitHub releases for updates