#include "LEDStripController.h"

LEDStripController::LEDStripController() {
    leds = new CRGB[LED_COUNT];
}

void LEDStripController::setup() {
    FastLED.addLeds<WS2812, LED_DATA_PIN, RGB>(leds, LED_COUNT);
    fill(10, 10, 10);
}

void LEDStripController::fill(int r, int g, int b) {
    fill_solid(leds, LED_COUNT, CRGB(r, g, b));
    show();
}

void LEDStripController::setLedColor(int ledIndex, int r, int g, int b) {
    if (ledIndex >= 0 && ledIndex < LED_COUNT) {
        leds[ledIndex] = CRGB(r, g, b);
    }
}

void LEDStripController::setColor(const int rgbValues[]) {
    for (int i = 0; i < LED_COUNT; i++) {
        int idx = i * 3;
        leds[i] = CRGB(rgbValues[idx], rgbValues[idx + 1], rgbValues[idx + 2]);
    }
    show();
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
