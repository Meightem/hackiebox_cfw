#include "Hackiebox.h"
#include "crashCallback.h"

BoxConfig Config;
BoxEvents Events;
Hackiebox Box;


void _SlDrvHandleGeneralEvents(SlDeviceEvent_t *pSlDeviceEvent) {
    Log.error("Received _SlDrvHandleGeneralEvents Event=%i", pSlDeviceEvent->Event);
}
int Report(const char *format, ...) {
    //Workaround for defined Report in the WiFi/utility/
    va_list args;
	va_start(args, format);
    Log.printFormat(format, args);
    return 0;
}
void crash(crashSource source, uint32_t* sp) {
    //Box.logStreamSse.setSsePaused(true);
    Log.info("crashSource=%i, sp=%X, sp=%X", source, sp, (uint32_t)sp-0x20004000);

    if (source == CRASH_TEST)
      return;

    Box.boxPower.feedSleepTimer();

    FileFs _file;
    bool _isOpen;
    Log.info("Dumping SRAM 0x20004000-0x20040000 to /revvox/memdump...");
    _isOpen = _file.open("/revvox/memdump", FA_CREATE_ALWAYS | FA_WRITE);
    if (_isOpen) {
        _file.write((void *)0x20004000, 0x3C000);
        _file.close();
    }
    Log.info("...done");
    Log.info("Dumping REGISTERS 0xE000E000-0xE000F000 to /revvox/regdump...");
    _isOpen = _file.open("/revvox/regdump", FA_CREATE_ALWAYS | FA_WRITE);
    if (_isOpen) {
        _file.write((void *)0xE000E000, 0x1000);
        _file.close();
    }
    Log.info("...done");

    Box.logStreamMulti.flush(); //Write all buffered logs to SD/SSE 
    
    __asm__ volatile("bkpt");
    
    Box.boxPower.hibernate();
}
void Hackiebox::setup() {  
    if (!watchdog_start(10)) {
        watchdog_stop();
        //reset box?!
    }
    uint32_t stackCanaries = countStackCanaries();
    uint32_t heapCanaries = countHeapCanaries();
    setCanaries();
    
    inDelayTask = true;
    logStreamMulti.setSlot(&logStreamSd, 0);
    logStreamMulti.setSlot(&logStreamSse, 1);
    Log.init(LOG_LEVEL_VERBOSE, 921600, &logStreamMulti);
    Log.info("Booting Hackiebox...");
    Box.boxPower.feedSleepTimer();
    Log.info("  -sizes: stack=%X, heap=%X", stackStart()-stackEnd(), heapEnd()-heapStart());
    Log.info("  -prev. canaries: stack=%ix, heap=%ix", stackCanaries, heapCanaries);

    register_crash_callback(crash);
    crashed(CRASH_TEST, 0);

    Wire.begin();
    
    boxPower.initPins();
    boxPower.setSdPower(true);
    boxPower.setOtherPower(true);

    boxSD.begin();
    Config.begin(); //SD Card needed!
    ConfigStruct* config = Config.get();

    watchdog_start(config->misc.watchdogSeconds);
    
    boxPower.begin();
    boxI2C.begin();
    boxLEDs.begin(config->misc.swd);
    boxLEDs.setAll(BoxLEDs::CRGB::White);
    boxBattery.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Orange);
    boxEars.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Yellow);
    boxAccel.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Pink);
    boxRFID.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Teal);
    //boxDAC.begin();
    boxLEDs.setAll(BoxLEDs::CRGB::Fuchsia);
    
    boxCLI.begin();

    Box.boxPower.feedSleepTimerSilent();
    boxWiFi = WrapperWiFi(config->wifi.ssid, config->wifi.password);
    boxWiFi.begin();
    Box.boxPower.feedSleepTimerSilent();

    webServer = WrapperWebServer();
    webServer.begin();

    boxPlayer = BoxPlayer();
    boxPlayer.begin();

    mqttHandler = MQTTHandler(config->mqtt.hostname, config->mqtt.port, config->mqtt.username, config->mqtt.password);
    mqttHandler.begin();
    
    boxAccel.setName("Accelerometer");
    boxBattery.setName("Battery");
    boxBattery.batteryTestThread.setName("Battery.Test");
    boxCLI.setName("CLI");
    boxDAC.setName("DAC");
    boxRFID.setName("RFID");
    boxEars.setName("Ears");
    boxLEDs.setName("LEDs");
    boxPower.setName("Power");
    boxWiFi.setName("WiFi");
    webServer.setName("Webserver");
    mqttHandler.setName("MQTTHandler");
    
    boxDAC.priority = 0;
    boxRFID.priority = 1;
    boxAccel.priority = 2;
    boxLEDs.priority = 3;
    boxEars.priority = 4;
    mqttHandler.priority = 5;
    webServer.priority = 6;
    boxPower.priority = 10;
    boxWiFi.priority = 50;
    boxBattery.priority = 100;
    boxBattery.batteryTestThread.priority = 100;
    boxCLI.priority = 100;

    threadController = ThreadController();
    threadController.add(&boxAccel);
    threadController.add(&boxBattery);
    threadController.add(&boxBattery.batteryTestThread);
    threadController.add(&boxCLI);
    threadController.add(&boxDAC);
    threadController.add(&boxEars);
    threadController.add(&boxRFID);
    threadController.add(&boxLEDs);
    threadController.add(&boxPower);
    threadController.add(&boxWiFi);
    threadController.add(&mqttHandler);
    threadController.add(&webServer);

    threadController.sortThreads();

    Log.info("Config: %s", Config.getAsJson().c_str());

    boxAccel.onRun(ThreadCallbackHandler([&]() { boxAccel.loop(); }));
    boxBattery.onRun(ThreadCallbackHandler([&]() { boxBattery.loop(); }));
    boxBattery.batteryTestThread.onRun(ThreadCallbackHandler([&]() { boxBattery.doBatteryTestStep(); }));
    boxCLI.onRun(ThreadCallbackHandler([&]() { boxCLI.loop(); }));
    boxDAC.onRun(ThreadCallbackHandler([&]() { boxDAC.loop(); }));
    boxEars.onRun(ThreadCallbackHandler([&]() { boxEars.loop(); }));
    boxRFID.onRun(ThreadCallbackHandler([&]() { boxRFID.loop(); }));
    boxLEDs.onRun(ThreadCallbackHandler([&]() { boxLEDs.loop(); }));
    boxPower.onRun(ThreadCallbackHandler([&]() { boxPower.loop(); }));
    boxWiFi.onRun(ThreadCallbackHandler([&]() { boxWiFi.loop(); }));
    mqttHandler.onRun(ThreadCallbackHandler([&]() { mqttHandler.loop(); }));
    webServer.onRun(ThreadCallbackHandler([&]() { webServer.loop(); }));

    //logStreamSse.setSsePaused(false);
 
    boxLEDs.defaultIdleAnimation();
    Log.info("Hackiebox started!");
    Box.boxPower.feedSleepTimer();
    inDelayTask = false;

    boxDAC.begin();
    //Workaround, as something seems to interfere / remove the irq.
    //But box now crashes!
    boxDAC.i2sStartMicros = micros();
}

void Hackiebox::delayTask(uint16_t millis) {
     if (!inDelayTask) {
        inDelayTask = true;
        BoxTimer timer;
        timer.setTimer(millis);

        //Work start
        while (timer.isRunning()) {
            delayTaskWork(timer.getTimeTillEnd());
            timer.tick();
        }
        //Work end

        inDelayTask = false;
    } else {
        delay(millis);
    }
}
void Hackiebox::delayTaskWork(uint16_t millis) {
    //delay(millis);
    boxDAC.loop(millis);
    //if (millis > 100)
    //    Log.debug("Delay %i", millis);
    //boxDAC.generateZeroAudio(millis);
}

void Hackiebox::loop() {
    watchdog_feed();
    threadController.run();
}

bool Hackiebox::watchdog_isFed() {
    return _watchdog_fed;
}
void Hackiebox::watchdog_feed() {
    _watchdog_fed = true;
}
void Hackiebox::watchdog_unfeed() {
    _watchdog_fed = false;
}
void watchdog_handler() {
    if (Box.watchdog_isFed() || !Box.watchdog_enabled) {
        MAP_WatchdogIntClear(WDT_BASE);
        Box.watchdog_unfeed();
    }
}
bool Hackiebox::watchdog_start(uint8_t timeoutS) {
    watchdog_feed();
    if (timeoutS == 0) {
        watchdog_start(53);
        //watchdog_stop(); // Random watchdog triggers?!
        watchdog_enabled = false;
    } else {
        if (timeoutS > 53)
            timeoutS = 53; //otherwise uint32_t of WatchdogReloadSet will overflow.
        
        MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_RUN_MODE_CLK);
        MAP_WatchdogUnlock(WDT_BASE);
        MAP_IntPrioritySet(INT_WDT, INT_PRIORITY_LVL_1);
        MAP_WatchdogStallEnable(WDT_BASE); //Allow Debugging
        MAP_WatchdogIntRegister(WDT_BASE, watchdog_handler);
        MAP_WatchdogReloadSet(WDT_BASE, 80000000*timeoutS);
        MAP_WatchdogEnable(WDT_BASE);

        watchdog_enabled = true;
    }
    return MAP_WatchdogRunning(WDT_BASE);
}
void Hackiebox::watchdog_stop() {  
    MAP_WatchdogUnlock(WDT_BASE);
    MAP_WatchdogReloadSet(WDT_BASE, 0xFFFFFFFF); //set timer to high value
    MAP_WatchdogIntClear(WDT_BASE);
    MAP_WatchdogIntUnregister(WDT_BASE);

    watchdog_enabled = false;
}

