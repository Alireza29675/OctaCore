#ifndef ServoController_h
#define ServoController_h

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoController {
public:
    ServoController(int pin); // Constructor with servo pin
    void setup(); // Attach the servo and initialize it
    void loop(); // Should be called in the main loop to update servo position smoothly
    void setAngle(int angle); // Set the target angle for the servo
    int getAngle() const; // Get the current angle of the servo

private:
    Servo _servo; // The servo object
    int _pin; // The GPIO pin connected to the servo signal line
    int _targetAngle; // The desired target angle
    int _currentAngle; // The current angle of the servo
    unsigned long _lastUpdateTime; // The last time the servo was updated
};

#endif
