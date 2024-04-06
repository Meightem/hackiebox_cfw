#include "MQTTHandler.h"
#include "Hackiebox.h"

MQTTHandler::MQTTHandler(const char* hostname, const int port, const char* username, const char* password) {  
  _hostname = hostname;
  _port = port;
  _username = username;
  _password = password;
}

void MQTTHandler::begin() {  
  Log.info("Initalising MQTT");
  mqttClient.begin(wifiClient);
  mqttClient.setHost(_hostname, _port);

  while(!mqttClient.connect("toniebox", _username, _password) && counter < 5) {
    counter++;
    delay(1000);
  }

  publishConfig();

  Log.info("Finished initalising MQTT");

  mqttReady = true;
}

void MQTTHandler::loop() {  
  mqttClient.loop();
}


void MQTTHandler::publishConfig() {
  publishSensorDeviceConfig("orientation");
  publishSensorDeviceConfig("status");
  publishSensorDeviceConfig("tonie");
}

void MQTTHandler::publishOrientationState(String state) {
  if(mqttReady) mqttClient.publish("homeassistant/sensor/toniebox-orientation/state", state);
}

void MQTTHandler::publishStatusState(String state) {
  if(mqttReady) mqttClient.publish("homeassistant/sensor/toniebox-status/state", state);
}

void MQTTHandler::publishTonieState(String state) {
  if(mqttReady) mqttClient.publish("homeassistant/sensor/toniebox-tonie/state", state);
}

void MQTTHandler::publishSensorDeviceConfig(const char* deviceName) {
  char devicePath[64];
  snprintf(devicePath, 64, "homeassistant/sensor/toniebox-%s/config", deviceName);
  char config[384];
  fillSensorDeviceConfig(config, deviceName);
  mqttClient.publish(devicePath, config, true, 0);
}

void MQTTHandler::fillSensorDeviceConfig(char* configStr, const char* deviceName) {
  snprintf(configStr, 384, "{\"~\":\"homeassistant/sensor/toniebox-%s\",\"name\":\"Toniebox %s\",\"uniq_id\":\"toniebox-%s\",\"stat_t\":\"~/state\",\"device\":{\"name\":\"toniebox\",\"identifiers\":[\"toniebox\"]}}", deviceName, deviceName, deviceName);
}