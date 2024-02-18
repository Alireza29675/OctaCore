#ifndef ServoController_h
#define ServoController_h

#include <ESP32Servo.h>
#include <EEPROM.h> 

class ServoController {
public:
    ServoController(int pin, int eepromAddress);

    void setup();
    void loop();

    void loadAngleFromEEPROM();
    void saveAngleToEEPROM();

    void setTargetAngle(int angle);
    int getTargetAngle() const;

private:
    Servo _servo;
    int _pin;
    int _targetAngle;
    int _currentAngle;
    int _eepromAddress;
};

#endif
