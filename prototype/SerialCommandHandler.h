#ifndef SerialCommandHandler_h
#define SerialCommandHandler_h

#include "Arduino.h"
#include "ServoController.h"

class SerialCommandHandler {
  public:
    SerialCommandHandler(ServoController& servoMotor);
    void setup();
    void loop();
  
  private:
    ServoController& _servoMotor;
    String _inputString;        
    bool _stringComplete;  
};

#endif
