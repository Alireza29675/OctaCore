#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoController {
 public:
  explicit ServoController(uint8_t pin);

  void setup();
  void loop();

  void setAngle(int angle);
  int getAngle() const;
  int getTargetAngle() const;

 private:
  Servo _servo;
  uint8_t _pin;
  int _targetAngle;
  int _currentAngle;
  unsigned long _lastUpdateTime;
};
