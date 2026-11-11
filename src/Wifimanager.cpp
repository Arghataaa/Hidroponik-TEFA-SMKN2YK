
#include "Wifimanager.h"

WiFiManagerModule::WiFiManagerModule() {
    _connected = false;
}

bool WiFiManagerModule::begin() {
    Serial.println();
    Serial.println("================================");
    Serial.println("       WIFI MANAGER MAPPI32     ");
    Serial.println("================================");

    const char* apName = "Hidroponik-TEFA";
    const char* apPassword = "hidroponik123";

    _wifiManager.setConfigPortalTimeout(180);
    _wifiManager.setConnectTimeout(20);

    Serial.println("[WiFi] Menghubungkan ke jaringan...");

    _connected = _wifiManager.autoConnect(apName, apPassword);

    if (_connected) {
        Serial.println("[WiFi] Berhasil terhubung!");

        Serial.print("[WiFi] SSID: ");
        Serial.println(WiFi.SSID());

        Serial.print("[WiFi] IP Address: ");
        Serial.println(WiFi.localIP());

        Serial.print("[WiFi] Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");

        return true;
    }

    Serial.println("[WiFi] Gagal terhubung!");
    return false;
}

bool WiFiManagerModule::isConnected() {
    _connected = (WiFi.status() == WL_CONNECTED);
    return _connected;
}

String WiFiManagerModule::getIP() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }

    return "Tidak terhubung";
}

String WiFiManagerModule::getSSID() {
    if (isConnected()) {
        return WiFi.SSID();
    }

    return "Tidak terhubung";
}

void WiFiManagerModule::disconnect() {
    WiFi.disconnect();
    _connected = false;

    Serial.println("[WiFi] Koneksi diputus.");
}

void WiFiManagerModule::resetSettings() {
    _wifiManager.resetSettings();
    _connected = false;

    Serial.println("[WiFi] Konfigurasi WiFi berhasil dihapus.");
}
