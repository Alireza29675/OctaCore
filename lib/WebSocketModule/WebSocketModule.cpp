#include "WebSocketModule.h"

WebSocketModule::WebSocketModule(unsigned int port) : _webSocketServer(port) {}

void WebSocketModule::begin() {
  _webSocketServer.begin();
}

void WebSocketModule::loop() {
  _webSocketServer.loop();
}

bool WebSocketModule::sendBIN(uint8_t client, uint8_t* payload, size_t length) {
  return _webSocketServer.sendBIN(client, payload, length);
}

void WebSocketModule::onWebSocketEvent(void (*event)(uint8_t num, WStype_t type, uint8_t * payload, size_t length)) {
  _webSocketServer.onEvent(event);
}
