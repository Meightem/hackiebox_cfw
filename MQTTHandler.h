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
    void publishOrientationState(String state);
    
    private:
        const char* _hostname;
        int _port;
        const char* _username;
        const char* _password;
        MQTTClient mqttClient;
        WiFiClient wifiClient;
        String _json;
        String getDeviceConfig();
        
        unsigned long lastMillis;
        unsigned long counter;
};

#endif
