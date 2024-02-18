#ifndef Globals_h
#define Globals_h

#include "LEDStripController.h"
#include "ServoController.h"
#include "WiFiModule.h"
#include "WebSocketModule.h"

extern WiFiModule wifiModule;
extern WebSocketModule webSocketModule;
extern LEDStripController ledStrip;
extern ServoController servoMotor;

#endif
