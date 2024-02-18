#ifndef WiFiModule_h
#define WiFiModule_h

#include <WiFiManager.h>
#include <WiFi.h>

class WiFiModule {
  public:
    WiFiModule(const char* apName, const char* apPassword);
    void connect();
    void disconnect();
    void resetSettings();
    IPAddress getIPAddress();
    
  private:
    const char* _apName;
    const char* _apPassword;
};

#endif
