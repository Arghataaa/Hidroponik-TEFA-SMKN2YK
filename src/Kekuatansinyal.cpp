#include "KekuatanSinyal.h"

KekuatanSinyal::KekuatanSinyal() {}

bool KekuatanSinyal::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

int KekuatanSinyal::getRSSI(uint8_t samples) {
  if (!isConnected()) return -100;       // tidak terhubung = sinyal terburuk

  long total = 0;
  for (uint8_t i = 0; i < samples; i++) {
    total += WiFi.RSSI();
    delay(10);
  }
  return (int)(total / samples);
}

int KekuatanSinyal::getPersen(uint8_t samples) {
  return rssiKePersen(getRSSI(samples));
}

String KekuatanSinyal::getKategori(uint8_t samples) {
  return rssiKeKategori(getRSSI(samples));
}

void KekuatanSinyal::printInfo() {
  if (!isConnected()) {
    Serial.println("Sinyal WiFi: tidak terhubung");
    return;
  }
  int rssi = getRSSI();
  Serial.printf("Sinyal WiFi: %d dBm | %d %% | %s\n",
                rssi, rssiKePersen(rssi), rssiKeKategori(rssi).c_str());
}

int KekuatanSinyal::rssiKePersen(int rssi) {
  if (rssi <= -100) return 0;
  if (rssi >= -50)  return 100;
  return 2 * (rssi + 100);               // -100 dBm = 0%, -50 dBm = 100%
}

String KekuatanSinyal::rssiKeKategori(int rssi) {
  if (rssi >= -50) return "Sangat Baik";
  if (rssi >= -60) return "Baik";
  if (rssi >= -70) return "Cukup";
  if (rssi >= -80) return "Lemah";
  return "Sangat Lemah";
}

// Fungsi global pendukung agar main.cpp bisa langsung memanggil getWiFiSignalQuality()
String getWiFiSignalQuality() {
    KekuatanSinyal ks;
    return ks.getKategori();
}