#include "Globals.h"
#include "Config.h"

WiFiModule wifiModule(
    OctaEnv::DEVICE_NAME,
    OctaEnv::WIFI_SSID,
    OctaEnv::WIFI_PASSWORD,
    OctaEnv::SETUP_AP_PASSWORD);

WebSocketModule webSocketModule(WEBSOCKET_PORT);
LEDStripController ledStrip;
ServoController servoMotor(SERVO_DATA_PIN);
