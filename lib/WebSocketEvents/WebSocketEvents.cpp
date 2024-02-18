#include "WebSocketEvents.h"
#include "LEDStripController.h"

extern LEDStripController ledStrip;

#define LED_DATA_LENGTH LED_COUNT * 3
#define SERVO_DATA_LENGTH 1

#define TOTAL_EXPECTED_LENGTH SERVO_DATA_LENGTH + LED_DATA_LENGTH

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_BIN) {
      if (length == TOTAL_EXPECTED_LENGTH) {
        updateLEDsFromPayload(payload);

        uint8_t servoAngle = payload[LED_COUNT * 3];
        setServoAngleFromPayload(servoAngle); 
      } else {
        Serial.println("Payload is corrupted!");
      }  
  }
}
