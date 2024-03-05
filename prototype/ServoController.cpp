#include "ServoController.h"

#define MIN_ANGLE 0
#define MAX_ANGLE 180 // Adjusted for 200 degrees
#define MAX_ANGLE_SPEED 1 // Incremental angle step for smoother movement
#define UPDATE_INTERVAL 10 // Time in milliseconds between servo updates for smooth movement

ServoController::ServoController(int pin) : _pin(pin), _lastUpdateTime(0) {
    _targetAngle = 90;
    _currentAngle = 90;
}

void ServoController::setup() {
    _servo.attach(_pin);
    _servo.write(_currentAngle); // Initialize servo position
}

void ServoController::loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - _lastUpdateTime >= UPDATE_INTERVAL) {
        _lastUpdateTime = currentMillis;
        
        if (_currentAngle != _targetAngle) {
            // Move towards the target angle in steps to prevent overshoot
            if (_currentAngle < _targetAngle) {
                _currentAngle += MAX_ANGLE_SPEED;
                if (_currentAngle > _targetAngle) _currentAngle = _targetAngle;
            } else if (_currentAngle > _targetAngle) {
                _currentAngle -= MAX_ANGLE_SPEED;
                if (_currentAngle < _targetAngle) _currentAngle = _targetAngle;
            }
            _servo.write(_currentAngle);
        }
    }
}

void ServoController::setAngle(int angle) {
    _targetAngle = constrain(angle, MIN_ANGLE, MAX_ANGLE); // Store target angle
}

int ServoController::getAngle() const {
    return _currentAngle; // Return the current angle, not the target
}
