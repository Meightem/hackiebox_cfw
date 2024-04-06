#include "BoxEvents.h"

void BoxEvents::begin() {

}
void BoxEvents::loop() {

}

void BoxEvents::handleEarEvent(BoxButtonEars::EarButton earId, BoxButtonEars::PressedType pressType, BoxButtonEars::PressedTime pressLength) {
    const char* nameEar;
    const char* nameType;
    const char* nameDuration;

    switch (earId) {
    case BoxButtonEars::EarButton::SMALL:
        nameEar = "small";
        break;
    case BoxButtonEars::EarButton::BIG:
        nameEar = "big";
        break;
    case BoxButtonEars::EarButton::BOTH:
        nameEar = "both";
        break;
    default:
        nameEar = "none";
        break;
    }

    switch (pressType) {
    case BoxButtonEars::PressedType::PRESS:
        nameType = "pressed";
        break;
    case BoxButtonEars::PressedType::RELEASE:
        nameType = "released";
        break;
    default:
        nameType = "?";
        break;
    }

    switch (pressLength) {
    case BoxButtonEars::PressedTime::SHORT:
        nameDuration = "short";
        break;
    case BoxButtonEars::PressedTime::LONG:
        nameDuration = "long";
        break;
    case BoxButtonEars::PressedTime::VERY_LONG:
        nameDuration = "verylong";
        break;
    default:
        nameDuration = "unknown";
        break;
    }


    Log.info("%s %s-%s", nameEar, nameDuration, nameType);
    Box.boxPower.feedSleepTimer();

    char earEvent[34];
    snprintf(earEvent, 34, "{\"id\":%i,\"type\":%i,\"duration\":%i}", earId, pressType, pressLength);
    Box.webServer.sendEvent("Ear", earEvent, false);

    if (pressType == BoxButtonEars::PressedType::PRESS) {
        if (pressLength == BoxButtonEars::PressedTime::SHORT) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                Box.boxDAC.increaseVolume();
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                Box.boxDAC.decreaseVolume();
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                
            }
        } else if (pressLength == BoxButtonEars::PressedTime::LONG) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                if (Box.boxWiFi.getStatus() != WrapperWiFi::ConnectionState::CONNECTED)
                    Box.boxWiFi.reconnect();
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                
            }
        } else if (pressLength == BoxButtonEars::PressedTime::VERY_LONG) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                if (Box.boxAccel.getOrientation() == BoxAccelerometer::Orientation::EARS_UP) {
                    Box.boxLEDs.setIdleAnimation(BoxLEDs::ANIMATION_TYPE::PULSE, BoxLEDs::CRGB::Blue);
                    Box.boxWiFi.apMode();
                } else if (Box.boxAccel.getOrientation() == BoxAccelerometer::Orientation::EARS_DOWN) {
                    Box.boxPower.reset();
                } else if (Box.boxAccel.getOrientation() == BoxAccelerometer::Orientation::EARS_FRONT) {
                    //Prepare Hibernation
                    Box.boxLEDs.setAllBool(false);
                    Box.boxEars.waitForRelease();
                    delay(500);
                    Box.boxPower.hibernate();
                }
            }
        }
    } else if (pressType == BoxButtonEars::PressedType::RELEASE) {
        char earState[24];
        snprintf(earEvent, 24, "%s-%s", nameEar, nameDuration);
        Box.mqttHandler.publishEarsState(earEvent);
        if (pressLength == BoxButtonEars::PressedTime::SHORT) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                
            }
        } else if (pressLength == BoxButtonEars::PressedTime::LONG) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                if (Box.boxWiFi.getStatus() != WrapperWiFi::ConnectionState::CONNECTED)
                    Box.boxWiFi.reconnect();
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {
                
            }
        } else if (pressLength == BoxButtonEars::PressedTime::VERY_LONG) {
            if (earId == BoxButtonEars::EarButton::BIG) {
                
            } else if (earId == BoxButtonEars::EarButton::SMALL) {
                
            } else if (earId == BoxButtonEars::EarButton::BOTH) {

            }
        }
    }
}

void BoxEvents::handleBatteryEvent(BoxBattery::BatteryEvent state) {
    char batteryEvent[12];
    snprintf(batteryEvent, 12, "{\"state\":%i}", state);
    Box.webServer.sendEvent("Battery", batteryEvent, false);

    switch (state) {
    case BoxBattery::BatteryEvent::BAT_CRITICAL:
        Log.info("Battery is critical, connect the charger, hibernating!");
        Box.boxBattery.stopBatteryTest();
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Orange, 3);
        Box.mqttHandler.publishBatterieState("critical");
        Box.boxLEDs.waitForAnimationToFinish();
        Box.boxPower.hibernate();
        break;
    case BoxBattery::BatteryEvent::BAT_LOW:
        Log.info("Battery is low, connect the charger!");
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setIdleAnimation(BoxLEDs::ANIMATION_TYPE::PULSE, BoxLEDs::CRGB::Orange);
        Box.mqttHandler.publishBatterieState("low");
        break;
    case BoxBattery::BatteryEvent::CHR_CONNECT:
        Log.info("Charger connected");
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::White, 3);
        Box.mqttHandler.publishChargerState("connected");
        break;
    case BoxBattery::BatteryEvent::CHR_DISCONNECT:
        Log.info("Charger disconnected");
        Box.boxBattery.logBatteryStatus();
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::DarkSlateGray, 3);
        Box.mqttHandler.publishChargerState("disconnected");
        break;
    }
}

void BoxEvents::handleWiFiEvent(WrapperWiFi::ConnectionState state) {
    switch (state) {
    case WrapperWiFi::ConnectionState::WAIT_CONNECT:
        break;
    case WrapperWiFi::ConnectionState::WAIT_IP:
        Log.info("WiFi connected, waiting for ip...");
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Cyan, 3);
        break;
    case WrapperWiFi::ConnectionState::CONNECTED:
        Log.info("IP: %s", WiFi.localIP().toString().c_str());
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Blue, 3);
        Box.boxWiFi.mDnsAdvertiseSetup();
        break;
    case WrapperWiFi::ConnectionState::DISCONNECTED:
        Box.boxLEDs.setActiveAnimationByIteration(BoxLEDs::ANIMATION_TYPE::BLINK, BoxLEDs::CRGB::Red, 3);
        Log.info("WiFi lost");
        break;
    default:
        break;
    }
}

void BoxEvents::handlePowerEvent(BoxPower::PowerEvent event) {
    switch (event)
    {
    case BoxPower::PowerEvent::PRE_HIBERNATE:
        Log.info("Go into hibernation...");
        break;
    case BoxPower::PowerEvent::PRE_RESET:
        Log.info("Reset box...");
        break;
    case BoxPower::PowerEvent::BOX_IDLE:
        Box.mqttHandler.publishStatusState("idle");
        break;
    case BoxPower::PowerEvent::BOX_ACTIVE:
        Box.mqttHandler.publishStatusState("active");
        break;
    default:
        break;
    }
}

void BoxEvents::handleAccelerometerTapsEvent(BoxAccelerometer::TapOn tap) {
    char* tapStr;
    switch (tap)
    {
    case BoxAccelerometer::TapOn::LEFT:
        tapStr = "LEFT";
        break;
    
    case BoxAccelerometer::TapOn::RIGHT:
        tapStr = "RIGHT";
        break;
    
    case BoxAccelerometer::TapOn::FRONT:
         tapStr = "FRONT";
        break;
    
    case BoxAccelerometer::TapOn::BACK:
         tapStr = "BACK";
        break;
    
    case BoxAccelerometer::TapOn::TOP:
         tapStr = "TOP";
        break;
    
    case BoxAccelerometer::TapOn::BOTTOM:
         tapStr = "BOTTOM";
        break;

    case BoxAccelerometer::TapOn::LEFT_FRONT:
         tapStr = "LEFT_FRONT";
        break;
    
    case BoxAccelerometer::TapOn::RIGHT_FRONT:
         tapStr = "RIGHT_FRONT";
        break;
    
    case BoxAccelerometer::TapOn::LEFT_BACK:
         tapStr = "LEFT_BACK";
        break;
    
    case BoxAccelerometer::TapOn::RIGHT_BACK:
         tapStr = "RIGHT_BACK";
        break;
    
    default:
        break;
        tapStr = "OTHER";
    }

    Box.mqttHandler.publishTapState(tapStr);
    Box.boxPower.feedSleepTimer();
}


void BoxEvents::handleAccelerometerOrientationEvent(BoxAccelerometer::Orientation orient) {
    const char* orientText;
    switch (orient) {
    case BoxAccelerometer::Orientation::EARS_UP:
        orientText = "ears up";
        Box.boxDAC.play();
        break;
    case BoxAccelerometer::Orientation::EARS_DOWN:
        orientText = "ears down";
        break;
    case BoxAccelerometer::Orientation::EARS_FRONT:
        orientText = "ears front";
        Box.boxDAC.pause();
        break;
    case BoxAccelerometer::Orientation::EARS_BACK:
        orientText = "ears back";
        break;
    case BoxAccelerometer::Orientation::EARS_LEFT:
        orientText = "ears left";
        break;
    case BoxAccelerometer::Orientation::EARS_RIGHT:
        orientText = "ears right";
        break;
    case BoxAccelerometer::Orientation::OTHER:
        orientText = "lockout";
        break;
    
    default:
        orientText = "unknown";
        Log.error("Unknown orientation=%i", orient);
        break;
    }
    Log.info("Box' orientation changed to %s", orientText);
    
    char orientEvent[9];
    snprintf(orientEvent, 9, "{\"id\":%i}", orient);
    Box.webServer.sendEvent("Orientation", orientEvent, false);
    Box.mqttHandler.publishOrientationState(orientText);

    Box.boxPower.feedSleepTimer();
}

void BoxEvents::handleTagEvent(BoxRFID::TAG_EVENT event) {
    uint8_t uid[24];
    uid[0] = '\0';

    switch (event) { 
    case BoxRFID::TAG_EVENT::TAG_PLACED:
        Log.info("Tag placed", event);
        Box.boxRFID.logUID();
        Box.boxRFID.getUID(uid);
        char uidStr[24];
        snprintf(uidStr, 24, "%s", uid);
        Box.mqttHandler.publishTonieState(uidStr);
        break;
    case BoxRFID::TAG_EVENT::TAG_REMOVED:
        Log.info("Tag removed", event);
        Box.mqttHandler.publishTonieState("");
        break;
    default:
        Log.error("Unknown TAG_EVENT=%X", event);
        break;
    }

    Box.boxPower.feedSleepTimer();
}

void BoxEvents::handleHeadphoneEvent(BoxDAC::HeadphoneEvent event) {
    switch (event){
    case BoxDAC::HeadphoneEvent::INSERTED:
        Log.info("Headphones connected");
        Box.boxDAC.muteSpeaker(true);
        Box.boxDAC.muteHeadphones(false);
        break;
    case BoxDAC::HeadphoneEvent::REMOVED:
        Log.info("Headphones disconnected");
        Box.boxDAC.muteHeadphones(true);
        Box.boxDAC.muteSpeaker(false);
        break;
    }
    char hpEvent[12];
    snprintf(hpEvent, 12, "{\"event\":%i}", hpEvent);
    Box.webServer.sendEvent("Headphones", hpEvent, false);
}