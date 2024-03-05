#ifndef Globals_h
#define Globals_h

#include "LEDStripController.h"
#include "ServoController.h"
#include "WiFiModule.h"
#include "WebSocketModule.h"
#include "SerialCommandHandler.h"

extern WiFiModule wifiModule;
extern WebSocketModule webSocketModule;
extern LEDStripController ledStrip;
extern ServoController servoMotor;
extern SerialCommandHandler serialCommandHandler;

#endif
