#ifndef LEDStripController_h
#define LEDStripController_h

#include <FastLED.h>
#include "Config.h"

class LEDStripController {
public:
    LEDStripController();
    void setup();
    void fill(uint8_t brushIndex);
    void setLedColor(int ledIndex, uint8_t brushIndex);
    void setColorPalette(const uint32_t colors[16]);
    void setBrightness(uint8_t brightness);
    void clear();
    void show();

private:
    CRGB* leds;
    CRGB colorPalette[16]; // Array to hold 16 brush colors
};

#endif
