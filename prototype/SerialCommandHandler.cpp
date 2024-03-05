#include "SerialCommandHandler.h"

SerialCommandHandler::SerialCommandHandler(ServoController& servoMotor) : _servoMotor(servoMotor), _stringComplete(false) {
}

void SerialCommandHandler::setup() {
  Serial.println("Enter servo command in format 'servo:angle' (e.g., 'servo:90')");
  _inputString = "";
}

void SerialCommandHandler::loop() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    _inputString += inChar;
    if (inChar == '\n') {
      _stringComplete = true;
    }
  }

  // Controlling Servo via Serial Commands
  if (_stringComplete) {
    if (_inputString.startsWith("servo:")) {
      int angle = _inputString.substring(6).toInt();
      _servoMotor.setAngle(angle);
      Serial.print("Moving servo to ");
      Serial.println(angle);
    }

    _inputString = "";
    _stringComplete = false;
  }
}
