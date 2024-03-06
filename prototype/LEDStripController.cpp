#include "LEDStripController.h"

LEDStripController::LEDStripController() {
    leds = new CRGB[LED_COUNT];
    for (int i = 0; i < 16; ++i) {
        colorPalette[i] = CRGB::Red;
    }
}

void LEDStripController::setup() {
    FastLED.addLeds<WS2812, LED_DATA_PIN, RGB>(leds, LED_COUNT);
    fill(0);
}

void LEDStripController::fill(uint8_t paletteIndex) {
    CRGB color = colorPalette[paletteIndex];
    fill_solid(leds, LED_COUNT, color);
    show();
}

void LEDStripController::setLedColor(int ledIndex, uint8_t paletteIndex) {
    if (ledIndex >= 0 && ledIndex < LED_COUNT) {
        CRGB color = colorPalette[paletteIndex];
        leds[ledIndex] = color;
    }
}

void LEDStripController::setColorPalette(const uint32_t colors[16]) {
    for (int i = 0; i < 16; ++i) {
        colorPalette[i] = CRGB(colors[i]);
    }
}

void LEDStripController::setBrightness(uint8_t brightness) {
    FastLED.setBrightness(brightness);
}

void LEDStripController::clear() {
    fill_solid(leds, LED_COUNT, CRGB::Black);
    show();
}

void LEDStripController::show() {
    FastLED.show();
}
