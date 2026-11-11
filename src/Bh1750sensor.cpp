#include "Bh1750sensor.h"
#include <Wire.h>
#include <BH1750.h>

// Membuat objek sensor
BH1750 lightMeter;

// Inisialisasi sensor BH1750
bool initBH1750() {

    if (lightMeter.begin(
        BH1750::CONTINUOUS_HIGH_RES_MODE)) {

        Serial.println("BH1750 berhasil terhubung");

        return true;

    } else {

        Serial.println("BH1750 tidak terdeteksi");

        return false;
    }
}

// Membaca intensitas cahaya
float readBH1750() {

    float lux = lightMeter.readLightLevel();

    return lux;
}
