#ifndef WebSocketModule_h
#define WebSocketModule_h

#include <WebSocketsServer.h>

class WebSocketModule {
  public:
    WebSocketModule(unsigned int port);
    void begin();
    void loop();
    void onWebSocketEvent(void (*event)(uint8_t num, WStype_t type, uint8_t * payload, size_t length));
    
  private:
    WebSocketsServer _webSocketServer;
};

#endif
