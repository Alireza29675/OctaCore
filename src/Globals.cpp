#include "Globals.h"
#include "WiFiCredentials.h"

WiFiModule wifiModule(WIFI_SSID, WIFI_PASSWORD);
WebSocketModule webSocketModule(81);
LEDStripController ledStrip;
