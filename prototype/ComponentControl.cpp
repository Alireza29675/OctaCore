#include "ComponentControl.h"
#include "WebSocketEvents.h" 

void updateLEDsFromPayload(uint8_t *payload) {
  for (int i = 0; i < LED_COUNT; i++) {
      int r = payload[i * 3]; // Red
      int g = payload[i * 3 + 1]; // Green
      int b = payload[i * 3 + 2]; // Blue

      ledStrip.setLedColor(i, r, g, b);
    }

    ledStrip.show();
}

void setServoAngleFromPayload(uint8_t angle) {
    servoMotor.setTargetAngle(angle); 
}
