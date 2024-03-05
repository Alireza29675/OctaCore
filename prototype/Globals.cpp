#include "Globals.h"

WiFiModule wifiModule("vector_module_1", "u-i.8Xwa8gX@e");
WebSocketModule webSocketModule(81);
LEDStripController ledStrip;
ServoController servoMotor(SERVO_DATA_PIN);
SerialCommandHandler serialCommandHandler(servoMotor);