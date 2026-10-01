#ifndef PaletteProtocol_h
#define PaletteProtocol_h

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "OctaCoreConfig.h"

// Validate complete frames before changing state.
struct PaletteProtocol {
    uint8_t palette[16][3] = {};
    uint8_t pixels[LED_COUNT][3] = {};
    uint8_t brightness = 100;

    bool apply(const uint8_t* payload, size_t length) {
        if (payload == nullptr || length == 0) return false;
        switch (payload[0]) {
            case 1: // Legacy servo command, deliberately ignored.
                return length == 2;
            case 2:
                if (length != 49) return false;
                memcpy(palette, payload + 1, sizeof(palette));
                return true;
            case 3:
                if (length != 1 + LED_COUNT / 2) return false;
                for (size_t i = 0; i < LED_COUNT; ++i) {
                    const uint8_t packed = payload[1 + i / 2];
                    const uint8_t index = i % 2 == 0 ? packed & 15 : packed >> 4;
                    memcpy(pixels[i], palette[index], 3);
                }
                return true;
            case 4:
                if (length != 2 || payload[1] >= 16) return false;
                for (size_t i = 0; i < LED_COUNT; ++i) {
                    memcpy(pixels[i], palette[payload[1]], 3);
                }
                return true;
            case 5:
                if (length != 2) return false;
                brightness = payload[1];
                return true;
            default:
                return false;
        }
    }
};

#endif
