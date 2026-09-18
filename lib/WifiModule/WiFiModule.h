#pragma once

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiManager.h>

class WiFiModule {
 public:
  WiFiModule(
      const char* deviceName,
      const char* wifiSsid,
      const char* wifiPassword,
      const char* setupApPassword);

  void connect();
  void disconnect();
  void resetSettings();
  void startMdns(uint16_t servicePort);

  IPAddress getIPAddress() const;

 private:
  bool connectDirect();

  const char* _deviceName;
  const char* _wifiSsid;
  const char* _wifiPassword;
  const char* _setupApPassword;
};
