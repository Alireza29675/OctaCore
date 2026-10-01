#ifndef WebSocketEvents_h
#define WebSocketEvents_h

#include <Arduino.h>
#include <WebSocketsServer.h>

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
void renderClubFrame();

#endif
