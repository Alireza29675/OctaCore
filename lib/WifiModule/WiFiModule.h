#ifndef WiFiModule_h
#define WiFiModule_h

#include <WiFi.h>

class WiFiModule {
  public:
    WiFiModule(const char* ssid, const char* password);
    void connect();
    void loop();
    IPAddress getIPAddress();

  private:
    const char* _ssid;
    const char* _password;
    bool _mdnsStarted = false;
    unsigned long _lastAttempt = 0;
    unsigned long _lastMdnsAttempt = 0;
};

#endif
