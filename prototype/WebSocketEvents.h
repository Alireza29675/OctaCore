#ifndef WebSocketEvents_h
#define WebSocketEvents_h

#include <Arduino.h>
#include <WebSocketsServer.h>
#include "Globals.h"
#include "Config.h"
#include "ComponentControl.h"

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

#endif
