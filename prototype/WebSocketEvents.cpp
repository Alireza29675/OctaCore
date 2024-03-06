#include "WebSocketEvents.h"

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_BIN) {
        if (length < 1) return;

        uint8_t commandId = payload[0] >> 5; // Extract the first 3 bits for the commandId

        switch (commandId) {
            case 0: // Command 000: Rotate servo
            {
                int angle = (payload[0] & 0x1F) << 3 | (payload[1] >> 5);
                servoMotor.setAngle(angle); 
                break;
            }
            case 1: // Command 001: Set brush colors
            {
                if (length < 25) return;
                uint32_t colors[8];
                for (int i = 0; i < 8; i++) {
                    uint32_t color = (payload[1 + i*3] << 16) | (payload[2 + i*3] << 8) | payload[3 + i*3];
                    colors[i] = color;
                    Serial.print(i);
                    Serial.print(": ");
                    Serial.print("(R:");
                    Serial.print((color >> 16) & 0xFF); // Extract the red component
                    Serial.print(", G:");
                    Serial.print((color >> 8) & 0xFF); // Extract the green component
                    Serial.print(", B:");
                    Serial.print(color & 0xFF); // Extract the blue component
                    Serial.println(")");
                }
                ledStrip.setColorPalette(colors);
                break;
            }
            case 2: // Command 010: Color LED strip by index
            {
                int numIndices = (length - 1) * 8 / 3;
                for (int ledIndex = 0; ledIndex < numIndices && ledIndex < LED_COUNT; ledIndex++) {
                    int byteIndex = 1 + (ledIndex * 3) / 8;
                    int bitOffset = (ledIndex * 3) % 8;
                    uint8_t brushIndex;
                    if (bitOffset <= 5) {
                        brushIndex = (payload[byteIndex] >> (5 - bitOffset)) & 0x07;
                    } else {
                        brushIndex = ((payload[byteIndex] << (3 + (8 - bitOffset))) | (payload[byteIndex + 1] >> (bitOffset - 5))) & 0x07;
                    }
                    Serial.print(brushIndex);
                    ledStrip.setLedColor(ledIndex, brushIndex);
                }
                Serial.println();
                // Set remaining LEDs to brush 0
                for (int ledIndex = numIndices; ledIndex < LED_COUNT; ledIndex++) {
                    ledStrip.setLedColor(ledIndex, 0);
                }
                ledStrip.show();
                break;
            }
        }
    }
}
