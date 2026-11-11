#ifndef KEKUATANSINYAL_H
#define KEKUATANSINYAL_H

#include <Arduino.h>
#include <WiFi.h>

class KekuatanSinyal {
public:
  KekuatanSinyal();

  bool   isConnected() const;
  int    getRSSI(uint8_t samples = 5);   // dBm, rata-rata beberapa sampel
  int    getPersen(uint8_t samples = 5); // 0-100 %
  String getKategori(uint8_t samples = 5);
  void   printInfo();                    // tampilkan ke Serial Monitor

private:
  static int rssiKePersen(int rssi);
  static String rssiKeKategori(int rssi);
};

#endif