#include "LEDStripController.h"

LEDStripController::LEDStripController() {
  for (uint8_t i = 0; i < COLOR_PALETTE_SIZE; ++i) {
    _colorPalette[i] = CRGB::Black;
  }
}

void LEDStripController::setup() {
  FastLED.addLeds<WS2812, LED_DATA_PIN, RGB>(_leds, LED_COUNT);
  FastLED.setBrightness(DEFAULT_LED_BRIGHTNESS);
  clear();
  show();
}

bool LEDStripController::isValidPaletteIndex(uint8_t paletteIndex) const {
  return paletteIndex < COLOR_PALETTE_SIZE;
}

void LEDStripController::fill(uint8_t paletteIndex) {
  if (!isValidPaletteIndex(paletteIndex)) {
    return;
  }

  fill_solid(_leds, LED_COUNT, _colorPalette[paletteIndex]);
}

void LEDStripController::setLedColor(
    uint16_t ledIndex,
    uint8_t paletteIndex) {
  if (ledIndex >= LED_COUNT || !isValidPaletteIndex(paletteIndex)) {
    return;
  }

  _leds[ledIndex] = _colorPalette[paletteIndex];
}

void LEDStripController::setColorPalette(
    const uint32_t colors[COLOR_PALETTE_SIZE]) {
  for (uint8_t i = 0; i < COLOR_PALETTE_SIZE; ++i) {
    _colorPalette[i] = CRGB(colors[i]);
  }
}

void LEDStripController::setBrightness(uint8_t brightness) {
  FastLED.setBrightness(brightness);
}

void LEDStripController::clear() {
  fill_solid(_leds, LED_COUNT, CRGB::Black);
}

void LEDStripController::show() {
  FastLED.show();
}
