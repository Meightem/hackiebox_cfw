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
}

void MQTTHandler::loop() {  
  mqttClient.loop();
}


void MQTTHandler::publishConfig() {
  mqttClient.publish("homeassistant/sensor/toniebox-orientation/config", getDeviceConfig(), true, 0);
}

void MQTTHandler::publishOrientationState(String state) {
  mqttClient.publish("homeassistant/sensor/toniebox-orientation/state", state);
}

String MQTTHandler::getDeviceConfig() {
  StaticJsonDocument<384> doc;

  doc["~"] = "homeassistant/sensor/toniebox-orientation";
  doc["name"] = "Toniebox Orientation";
  doc["uniq_id"] = "toniebox-orientation";
  doc["stat_t"] = "~/state";
  doc["schema"] = "json";

  JsonObject device = doc.createNestedObject("device");
  device["name"] = "toniebox";
  JsonArray device_identifiers = device.createNestedArray("identifiers");
  device_identifiers.add("toniebox");

  _json = "";
  serializeJson(doc, _json);
  return _json;
}