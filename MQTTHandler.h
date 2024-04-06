#ifndef MQTTHandler_h
#define MQTTHandler_h

#include "BaseHeader.h"
#include <EnhancedThread.h>
#include <MQTT.h>

class MQTTHandler : public EnhancedThread { 
    public:
        MQTTHandler() {};
        MQTTHandler(const char* hostname, const int port, const char* username, const char* password);
    void
        begin(),
        loop();

    void publishConfig();
    void publishStatusState(String state);
    void publishOrientationState(String state);
    void publishTonieState(String state);
    
    private:
        const char* _hostname;
        int _port;
        const char* _username;
        const char* _password;
        MQTTClient mqttClient;
        WiFiClient wifiClient;
        String _json;
        void publishSensorDeviceConfig(const char* deviceName);
        void fillSensorDeviceConfig(char* configStr, const char* deviceName);
        unsigned long lastMillis;
        unsigned long counter;
        bool mqttReady;
};

#endif
