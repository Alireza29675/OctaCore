#include "WiFiModule.h"

WiFiModule::WiFiModule(const char* apName, const char* apPassword) {
  _apName = apName;
  _apPassword = apPassword;
}

void WiFiModule::connect() {
  WiFiManager wifiManager;

  if (!wifiManager.autoConnect(_apName, _apPassword)) {
    Serial.println("Failed to connect and hit timeout");
    ESP.restart();
    delay(1000);
  }

  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void WiFiModule::disconnect() {
  WiFi.disconnect();
  Serial.println("Disconnected from WiFi.");
}

void WiFiModule::resetSettings() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  Serial.println("WiFi settings reset.");
}

IPAddress WiFiModule::getIPAddress() {
  return WiFi.localIP();
}
