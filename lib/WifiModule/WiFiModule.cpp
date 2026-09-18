#include "WiFiModule.h"

#include "Config.h"

WiFiModule::WiFiModule(
    const char* deviceName,
    const char* wifiSsid,
    const char* wifiPassword,
    const char* setupApPassword)
    : _deviceName(deviceName),
      _wifiSsid(wifiSsid),
      _wifiPassword(wifiPassword),
      _setupApPassword(setupApPassword) {}

bool WiFiModule::connectDirect() {
  if (_wifiSsid == nullptr || strlen(_wifiSsid) == 0) {
    return false;
  }

  Serial.print("Connecting to configured Wi-Fi: ");
  Serial.println(_wifiSsid);

  WiFi.begin(_wifiSsid, _wifiPassword);

  const unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startedAt < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  return WiFi.status() == WL_CONNECTED;
}

void WiFiModule::connect() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(_deviceName);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  bool connected = connectDirect();

  if (!connected) {
    Serial.println("Using WiFiManager fallback.");

    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(180);

    if (_setupApPassword != nullptr && strlen(_setupApPassword) >= 8) {
      connected = wifiManager.autoConnect(_deviceName, _setupApPassword);
    } else {
      connected = wifiManager.autoConnect(_deviceName);
    }
  }

  if (!connected || WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi connection failed. Restarting...");
    delay(1000);
    ESP.restart();
  }

  Serial.println("Connected to Wi-Fi.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFiModule::disconnect() {
  WiFi.disconnect();
  Serial.println("Disconnected from Wi-Fi.");
}

void WiFiModule::resetSettings() {
  Serial.println("Resetting stored Wi-Fi settings...");

  WiFiManager wifiManager;
  wifiManager.resetSettings();
  WiFi.disconnect(true, true);

  delay(500);
  ESP.restart();
}

void WiFiModule::startMdns(uint16_t servicePort) {
  if (!MDNS.begin(_deviceName)) {
    Serial.println("mDNS startup failed.");
    return;
  }

  MDNS.addService("ws", "tcp", servicePort);

  Serial.print("mDNS: ws://");
  Serial.print(_deviceName);
  Serial.print(".local:");
  Serial.println(servicePort);
}

IPAddress WiFiModule::getIPAddress() const {
  return WiFi.localIP();
}
