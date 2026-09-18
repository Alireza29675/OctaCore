#pragma once

#include <FastLED.h>
#include "Config.h"

class LEDStripController {
 public:
  LEDStripController();

  void setup();
  void fill(uint8_t paletteIndex);
  void setLedColor(uint16_t ledIndex, uint8_t paletteIndex);
  void setColorPalette(const uint32_t colors[COLOR_PALETTE_SIZE]);
  void setBrightness(uint8_t brightness);
  void clear();
  void show();

 private:
  bool isValidPaletteIndex(uint8_t paletteIndex) const;

  CRGB _leds[LED_COUNT];
  CRGB _colorPalette[COLOR_PALETTE_SIZE];
};
