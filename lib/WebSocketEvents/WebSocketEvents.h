#pragma once

#include <Arduino.h>
#include <WebSocketsServer.h>

void onWebSocketEvent(
    uint8_t clientNumber,
    WStype_t type,
    uint8_t* payload,
    size_t length);
