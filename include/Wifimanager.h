#ifndef WIFI_MANAGER_MODULE_H
#define WIFI_MANAGER_MODULE_H

#include <Arduino.h>
#include <WiFiManager.h>

class WiFiManagerModule {
public:
    WiFiManagerModule();

    // Inisialisasi dan koneksi WiFi
    bool begin();

    // Mengecek status koneksi
    bool isConnected();

    // Mendapatkan IP ESP32
    String getIP();

    // Mendapatkan nama WiFi yang terhubung
    String getSSID();

    // Memutus koneksi WiFi
    void disconnect();

    // Menghapus konfigurasi WiFi tersimpan
    void resetSettings();

private:
    WiFiManager _wifiManager;
    bool _connected;
};

#endif