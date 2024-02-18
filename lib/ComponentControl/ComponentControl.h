#ifndef COMPONENTCONTROL_H
#define COMPONENTCONTROL_H

#include <stdint.h>
#include "Config.h"

void updateLEDsFromPayload(uint8_t *payload);
void setServoAngleFromPayload(uint8_t angle); 

#endif
