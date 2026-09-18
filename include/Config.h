#pragma once

#include <Arduino.h>
#include "EnvGenerated.h"

constexpr uint8_t LED_DATA_PIN = 5;
constexpr uint16_t LED_COUNT = 60;
constexpr uint8_t SERVO_DATA_PIN = 15;

constexpr uint8_t COLOR_PALETTE_SIZE = 16;
constexpr uint8_t DEFAULT_LED_BRIGHTNESS = 100;

constexpr uint8_t SERVO_MIN_ANGLE = 0;
constexpr uint8_t SERVO_MAX_ANGLE = 180;
constexpr uint8_t SERVO_START_ANGLE = 90;
constexpr uint8_t SERVO_STEP_DEGREES = 1;
constexpr uint16_t SERVO_UPDATE_INTERVAL_MS = 10;

constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr uint16_t WEBSOCKET_PORT = OctaEnv::WEBSOCKET_PORT;
