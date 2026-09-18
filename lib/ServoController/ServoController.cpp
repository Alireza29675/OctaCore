#include "ServoController.h"

#include "Config.h"

ServoController::ServoController(uint8_t pin)
    : _pin(pin),
      _targetAngle(SERVO_START_ANGLE),
      _currentAngle(SERVO_START_ANGLE),
      _lastUpdateTime(0) {}

void ServoController::setup() {
  _servo.attach(_pin);
  _servo.write(_currentAngle);
}

void ServoController::loop() {
  const unsigned long now = millis();
  if (now - _lastUpdateTime < SERVO_UPDATE_INTERVAL_MS) {
    return;
  }

  _lastUpdateTime = now;

  if (_currentAngle == _targetAngle) {
    return;
  }

  const int difference = _targetAngle - _currentAngle;
  const int step = constrain(
      abs(difference),
      1,
      static_cast<int>(SERVO_STEP_DEGREES));

  _currentAngle += difference > 0 ? step : -step;
  _currentAngle = constrain(
      _currentAngle,
      static_cast<int>(SERVO_MIN_ANGLE),
      static_cast<int>(SERVO_MAX_ANGLE));

  _servo.write(_currentAngle);
}

void ServoController::setAngle(int angle) {
  _targetAngle = constrain(
      angle,
      static_cast<int>(SERVO_MIN_ANGLE),
      static_cast<int>(SERVO_MAX_ANGLE));
}

int ServoController::getAngle() const {
  return _currentAngle;
}

int ServoController::getTargetAngle() const {
  return _targetAngle;
}
