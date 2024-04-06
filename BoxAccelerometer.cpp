#include "BoxAccelerometer.h"
#include "BoxEvents.h"

void BoxAccelerometer::begin() {  
    pinMode(62, OUTPUT);
    digitalWrite(62, HIGH);
    
    setInterval(250);
    Log.info("Init Accelerometer...");

    if (!_accel.begin())
        Log.error("...not found!");

    _accel.setupPL();

    
    _accel.writeRegister(CTRL_REG1, 0x02); //Original 0x02 //F_READ
    _accel.writeRegister(XYZ_DATA_CFG, 0x02); //Original 0x02 //FS1
    _accel.writeRegister(CTRL_REG2, 0x00); //Original 0x00 //Standby
    _accel.writeRegister(F_SETUP, 0x00); //Original 0x00
    _accel.writeRegister(TRIG_CFG, 0x08); //Original 0x08 //Trig_PULSE/ZSPEFE/ELE
    _accel.writeRegister(PULSE_CFG, 0x54); //Original 0x54 //YSPEFE
    _accel.setupTap(0x1B, 0x3F, 0x3F); //Original 0x1B, 0x3F, 0x3F
    _accel.writeRegister(PULSE_TMLT, 0x28); //Original 0x28 //TMLT3/TMLT5
    _accel.writeRegister(PULSE_LTCY, 0x7F); //Original 0x7F //LTCY6/LTCY5/LTCY4/LTCY3/LTCY2/LTCY1/LTCY0
    _accel.writeRegister(HP_FILTER_CUTOFF, 0x10); //Original 0x10 //Pulse_LPF_EN

    _accel.writeRegister(CTRL_REG3, 0x12); //Original 0x12 //WAKE_PULSE/IPOL
    _accel.writeRegister(CTRL_REG4, 0x40); //Original 0x40 //INT_EN_FIFO
    _accel.writeRegister(CTRL_REG5, 0x40); //Original 0x40 //INT_CFG_FIFO INT1
    _accel.writeRegister(CTRL_REG1, 0x03); //Original 0x03 //F_READ/ACTIVE

    Log.info("...done");
}

void BoxAccelerometer::loop() {  
    if (_accel.available()) { 
        //Log.verbose("Accelerometer=(%i, %i, %i), Orient=%i", _accel.getX(), _accel.getY(), _accel.getZ(), _accel.readOrientation());

        Orientation orient = (Orientation)_accel.readOrientation();
        if (orient == Orientation::EARS_UP2) {
            orient = Orientation::EARS_UP;
        } else if (orient == Orientation::EARS_DOWN2) {
            orient = Orientation::EARS_DOWN;
        }
        
        if (_orientation != orient) {
            _orientation = orient;
            Events.handleAccelerometerOrientationEvent(_orientation);
        }

        uint8_t tap = _accel.readTap();
        if (tap) {
            bool AxZ  = (tap&0b1000000)>0; //event on axis
            bool AxY  = (tap&0b0100000)>0;
            bool AxX  = (tap&0b0010000)>0;
            //bool DPE  = (tap&0b0001000)>0; //double
            bool PolZ = !((tap&0b0000100)>0); //0=positive 1=negative
            bool PolY = !((tap&0b0000010)>0);
            bool PolX = !((tap&0b0000001)>0);

            //X+ = box bottom
            //X- = box top
            //Y+ = box back left (big ear)
            //Y- = box front right (speaker, small ear)
            //Z+ = box back right (small ear)
            //Z- = box front left (speaker, big ear)

            //Something wrong, only blinks red or greenyellow
            TapOn tapOn = TapOn::NONE;
            if (AxX) {
                if (PolX) {
                    tapOn = TapOn::BOTTOM;
                } else {
                    tapOn = TapOn::TOP;
                }
            }
            if (AxY && AxZ) {
                if (PolY && PolZ) {
                    tapOn = TapOn::BACK;
                } else if (!PolY && !PolZ) {
                    tapOn = TapOn::FRONT;
                } else if (PolY && !PolZ) {
                    tapOn = TapOn::LEFT;
                } else if (!PolY && PolZ) {
                    tapOn = TapOn::RIGHT;
                }
            } else if (AxY) {
                if (PolY) {
                    tapOn = TapOn::LEFT_BACK;
                } else {
                    tapOn = TapOn::LEFT_FRONT;
                }
            } else if (AxZ) {
                if (PolZ) {
                    tapOn = TapOn::RIGHT_BACK;
                } else {
                    tapOn = TapOn::RIGHT_FRONT;
                }
            }

            Events.handleAccelerometerTapsEvent(tapOn);
        }
        
    }
}

void BoxAccelerometer::reloadConfig() {  

}

BoxAccelerometer::Orientation BoxAccelerometer::getOrientation() {
    return _orientation;
}