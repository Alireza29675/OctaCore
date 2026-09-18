#include "Config.h"
#include "Globals.h"
#include "WebSocketEvents.h"

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println("=== OctaCore boot ===");
  Serial.print("Device: ");
  Serial.println(OctaEnv::DEVICE_NAME);

  ledStrip.setup();
  servoMotor.setup();

  wifiModule.connect();
  wifiModule.startMdns(WEBSOCKET_PORT);

  webSocketModule.begin();
  webSocketModule.onWebSocketEvent(onWebSocketEvent);

  Serial.print("WebSocket listening on port ");
  Serial.println(WEBSOCKET_PORT);
}

void loop() {
  webSocketModule.loop();
  servoMotor.loop();
}
