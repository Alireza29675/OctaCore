#include "WebSocketEvents.h"

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_BIN) {
        if (length < 1) return;

        uint8_t commandId = payload[0];

        switch (commandId) {
            case 1: // Rotate servo
            {
                if (length < 2) return;
                int angle = payload[1];
                servoMotor.setAngle(angle); 
                break;
            }
            case 2: // Set color palette
            {
                int numColors = (length - 1) / 3; 
                uint32_t colors[16];
                
                for (int i = 0; i < numColors; i++) {
                    uint32_t color = (payload[1 + i*3] << 16) | (payload[2 + i*3] << 8) | payload[3 + i*3];
                    colors[i] = color;
                }
                
                // Fill the rest of the colors with black
                for (int i = numColors; i < 16; i++) {
                    colors[i] = 0x000000; // Black
                }
                
                ledStrip.setColorPalette(colors);
                break;
            }
            case 3: // Set LED color
            {
                int numLEDs = (length - 1) * 2;
                for (int ledIndex = 0; ledIndex < numLEDs && ledIndex < LED_COUNT; ledIndex++) {
                    int byteIndex = 1 + ledIndex / 2;
                    uint8_t ledData = payload[byteIndex];
                    
                    uint8_t paletteIndex;
                    if (ledIndex % 2 == 0) {
                        // even index: extract the first 4 bits
                        paletteIndex = (ledData >> 4) & 0x0F;
                    } else {
                        // odd index: extract the last 4 bits
                        paletteIndex = ledData & 0x0F;
                    }
                    
                    ledStrip.setLedColor(ledIndex, paletteIndex);
                }

                ledStrip.show();
                break;
            }
            case 4: // Fill LED strip
            {
                if (length < 2) return;
                uint8_t paletteIndex = payload[1];
                ledStrip.fill(paletteIndex);
                break;
            }
            case 5: // Set brightness
            {
                if (length < 2) return;
                uint8_t brightness = payload[1];
                ledStrip.setBrightness(brightness);
                break;
            }
        }
    }
}
