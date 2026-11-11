#pragma once

#include <Arduino.h>

// Pin I2C default ESP32 / Mappi32 (bisa di-override lewat config.h atau build_flags)
#ifndef AHT20_SDA_PIN
#define AHT20_SDA_PIN 21
#endif
    
#ifndef AHT20_SCL_PIN
#define AHT20_SCL_PIN 22
#endif

// Inisialisasi sensor. Return true jika sensor terdeteksi di alamat 0x38.
bool aht20Begin();

// Baca suhu (°C) dan kelembapan (%RH). Return false jika pembacaan gagal.
bool aht20Read(float &temperature, float &humidity);

// Versi singkat, return NAN jika gagal.
float aht20GetTemperature();
float aht20GetHumidity();

// Status inisialisasi sensor.
bool aht20IsReady();