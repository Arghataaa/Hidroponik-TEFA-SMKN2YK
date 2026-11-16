#include "Bh1750sensor.h"
#include <Wire.h>
#include <BH1750.h>

// Membuat objek sensor
BH1750 lightMeter;

// Inisialisasi sensor BH1750 pada bus I2C bersama LCD dan AHT20.
// Pin ADDR ke GND = 0x23, pin ADDR ke VCC = 0x5C.
bool initBH1750() {
    if (lightMeter.begin(
            BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire)) {
        Serial.println("BH1750 berhasil terhubung pada alamat 0x23");
        return true;
    }

    if (lightMeter.begin(
            BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C, &Wire)) {
        Serial.println("BH1750 berhasil terhubung pada alamat 0x5C");
        return true;
    }

    Serial.println("BH1750 tidak terdeteksi pada alamat 0x23 maupun 0x5C");
    return false;
}

// Membaca intensitas cahaya
float readBH1750() {
    return lightMeter.readLightLevel();
}
