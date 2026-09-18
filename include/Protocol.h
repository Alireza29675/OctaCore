#pragma once

#include <Arduino.h>

enum class OctaCoreCommand : uint8_t {
  RotateServo = 1,
  SetColorPalette = 2,
  SetLedColors = 3,
  FillLeds = 4,
  SetLedBrightness = 5,
  ResetWifiSettings = 15,
};
