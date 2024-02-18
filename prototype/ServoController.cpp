#include "ServoController.h"

#define MIN_ANGLE 0
#define MAX_ANGLE 180
#define MAX_ANGLE_SPEED 5

ServoController::ServoController(int pin, int eepromAddress) : _pin(pin), _eepromAddress(eepromAddress) { 
    _targetAngle = 0;
    _currentAngle = 0;
    loadAngleFromEEPROM();
}

void ServoController::setup() {
    _servo.attach(_pin);
}

void ServoController::loop() {
    if (_currentAngle != _targetAngle) {
      int difference = _targetAngle - _currentAngle;
      int rotateDirection = difference == 0 ? 0 : (difference > 0 ? 1 : -1);
      int rotate = rotateDirection * MAX_ANGLE_SPEED;

      if (abs(difference) < MAX_ANGLE_SPEED) {
          _currentAngle = _targetAngle;
      } else {
          _currentAngle += rotate;
      }

      _currentAngle = constrain(_currentAngle, MIN_ANGLE, MAX_ANGLE); 

      _servo.write(_currentAngle);
    } else {
      saveAngleToEEPROM();  
    }
}

void ServoController::setTargetAngle(int angle) {
    _targetAngle = angle;
}

int ServoController::getTargetAngle() const {
    return _targetAngle;
}  

void ServoController::loadAngleFromEEPROM() {
    _targetAngle = EEPROM.read(_eepromAddress);
    _currentAngle = _targetAngle; // Start from stored angle
}

void ServoController::saveAngleToEEPROM() {
    EEPROM.write(_eepromAddress, _currentAngle);
}
