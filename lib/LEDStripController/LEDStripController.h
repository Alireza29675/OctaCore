#ifndef LEDStripController_h
#define LEDStripController_h

#include <FastLED.h>
#include "Config.h"

class LEDStripController {
public:
    LEDStripController();
    void setup();
    void fill(int r, int g, int b);
    void setLedColor(int ledIndex, int r, int g, int b);
    void setColor(const int rgbValues[]);
    void setBrightness(uint8_t brightness);
    void clear();
    void show();

private:
    CRGB* leds;
};

#endif
