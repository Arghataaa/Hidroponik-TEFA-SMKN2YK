#include "Suhuairsensor.h"
#include <OneWire.h>

#define ONEWIRE_PIN 13 // GPIO 13 ESP32

// Inisialisasi objek OneWire
static OneWire ds(ONEWIRE_PIN);

void initSuhuAir() {
    // Dipanggil saat setup jika ada inisialisasi tambahan
}

float getTemp() {
    byte data[9];
    byte addr[8];

    // Cari alamat sensor DS18B20
    if (!ds.search(addr)) {
        ds.reset_search();
        return -1000;
    }

    // Validasi CRC dan tipe ROM DS18B20
    if (OneWire::crc8(addr, 7) != addr[7] || (addr[0] != 0x10 && addr[0] != 0x28)) {
        return -1000;
    }

    // Mulai proses konversi suhu
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);
    delay(750); // Tunggu pembacaan suhu selesai (750 ms)

    // Baca Scratchpad DS18B20
    ds.reset();
    ds.select(addr);
    ds.write(0xBE);

    for (int i = 0; i < 9; i++) {
        data[i] = ds.read();
    }

    ds.reset_search();

    // Hitung nilai suhu mentah (12-bit)
    int16_t raw = (data[1] << 8) | data[0];
    return (float)raw / 16.0;
}