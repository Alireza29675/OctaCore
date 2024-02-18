#include "Globals.h"

WiFiModule wifiModule(ACCESS_POINT_SSID, ACCESS_POINT_PASSWORD);
WebSocketModule webSocketModule(81);
LEDStripController ledStrip;
ServoController servoMotor(SERVO_DATA_PIN, 0);
