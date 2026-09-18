#pragma once

#include <Arduino.h>
#include "EnvGenerated.h"

inline constexpr uint8_t LED_DATA_PIN = 5;
inline constexpr uint16_t LED_COUNT = 60;
inline constexpr uint8_t SERVO_DATA_PIN = 15;

inline constexpr uint8_t COLOR_PALETTE_SIZE = 16;
inline constexpr uint8_t DEFAULT_LED_BRIGHTNESS = 100;

inline constexpr uint8_t SERVO_MIN_ANGLE = 0;
inline constexpr uint8_t SERVO_MAX_ANGLE = 180;
inline constexpr uint8_t SERVO_START_ANGLE = 90;
inline constexpr uint8_t SERVO_STEP_DEGREES = 1;
inline constexpr uint16_t SERVO_UPDATE_INTERVAL_MS = 10;

inline constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
inline constexpr uint16_t WEBSOCKET_PORT = OctaEnv::WEBSOCKET_PORT;
