#include "WebSocketEvents.h"
#include "Globals.h"

void setup() {
  Serial.begin(115200);
  
  ledStrip.setup();
  wifiModule.connect();
  webSocketModule.begin();
  webSocketModule.onWebSocketEvent(onWebSocketEvent);

}

void loop() {
  webSocketModule.loop();
  wifiModule.loop();
  renderClubFrame();
}
