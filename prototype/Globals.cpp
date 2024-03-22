#include "Globals.h"

WiFiModule wifiModule("octacore", "highTechMe$$");
WebSocketModule webSocketModule(81);
LEDStripController ledStrip;
ServoController servoMotor(SERVO_DATA_PIN);
SerialCommandHandler serialCommandHandler(servoMotor);