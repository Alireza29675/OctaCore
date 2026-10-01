#include "WiFiModule.h"
#include <ESPmDNS.h>
#include "TowerIdentity.h"

WiFiModule::WiFiModule(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {}

void WiFiModule::connect() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(OCTACORE_HOSTNAME);
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
  if (_ssid[0] == '\0') {
    Serial.println("Wi-Fi is not configured; staying offline.");
    return;
  }
  WiFi.begin(_ssid, _password);
  _lastAttempt = millis();
}

void WiFiModule::loop() {
  if (_ssid[0] == '\0') return;
  const unsigned long now = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (_mdnsStarted) {
      MDNS.end();
      _mdnsStarted = false;
    }
    if (now - _lastAttempt >= 30000) {
      WiFi.reconnect();
      _lastAttempt = now;
    }
    return;
  }
  if (!_mdnsStarted && now - _lastMdnsAttempt >= 5000) {
    _lastMdnsAttempt = now;
    if (MDNS.begin(OCTACORE_HOSTNAME)) {
      MDNS.addService("octacore", "tcp", 81);
      MDNS.addServiceTxt("octacore", "tcp", "role", OCTACORE_ROLE);
      MDNS.addServiceTxt("octacore", "tcp", "tower", OCTACORE_TOWER_LABEL);
      MDNS.addServiceTxt("octacore", "tcp", "protocol", "palette-v1");
      _mdnsStarted = true;
      Serial.print("Tower " OCTACORE_TOWER_LABEL " ready: " OCTACORE_HOSTNAME ".local at ");
      Serial.println(WiFi.localIP());
    }
  }
}

IPAddress WiFiModule::getIPAddress() {
  return WiFi.localIP();
}
