#include "WebSocketEvents.h"
#include "Globals.h"

void setup() {
  Serial.begin(115200);
  
  wifiModule.connect();
  webSocketModule.begin();
  webSocketModule.onWebSocketEvent(onWebSocketEvent);

  ledStrip.setup();
  servoMotor.setup();
}

void loop() {
  webSocketModule.loop();
  servoMotor.loop();
}
