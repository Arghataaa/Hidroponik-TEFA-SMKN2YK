#ifndef WIFI_MANAGER_MODULE_H
#define WIFI_MANAGER_MODULE_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h> // Sekarang ini aman memanggil library tzapu

class WiFiManagerModule {
public:
    WiFiManagerModule();

    bool begin();
    bool isConnected();

    String getIP();
    String getSSID();

    void disconnect();
    void resetSettings();

private:
    WiFiManager _wifiManager;
    bool _connected;
};

#endif