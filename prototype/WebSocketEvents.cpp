#include "WebSocketEvents.h"
#include "LEDStripController.h"

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_BIN) {
      updateLEDsFromPayload(payload);

      uint8_t servoAngle = payload[LED_COUNT * 3];
      setServoAngleFromPayload(servoAngle); 
  }
}
