#ifndef AHT20SENSOR_H
#define AHT20SENSOR_H

#include <Arduino.h>

// Pin I2C default ESP32 / Mappi32
// Bisa diubah melalui config.h atau build_flags
#ifndef AHT20_SDA_PIN
#define AHT20_SDA_PIN 21
#endif

#ifndef AHT20_SCL_PIN
#define AHT20_SCL_PIN 22
#endif

// Inisialisasi sensor AHT20.
// Return true jika sensor terdeteksi pada alamat 0x38.
bool aht20Begin();

// Membaca suhu (°C) dan kelembapan (%RH).
// Return false jika pembacaan gagal.
bool aht20Read(float &temperature, float &humidity);

// Membaca suhu. Return NAN jika gagal.
float aht20GetTemperature();

// Membaca kelembapan. Return NAN jika gagal.
float aht20GetHumidity();

// Memeriksa status inisialisasi sensor.
bool aht20IsReady();

#endif // AHT20SENSOR_H
