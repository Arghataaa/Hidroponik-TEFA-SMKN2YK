#include "pHsensor.h"
#include "DFRobot_PH.h"
#include <EEPROM.h>

#define PH_PIN 34

// Variabel internal modul pH
static DFRobot_PH ph;
static float currentVoltage = 0.0;

void initPH() {
    // Set resolusi & redaman ADC ESP32
    analogReadResolution(12);
    analogSetPinAttenuation(PH_PIN, ADC_11db);

    // Inisialisasi library pH DFRobot
    ph.begin();
}

float readPH(float temperature) {
    // 1. Baca tegangan analog dalam milivolt
    // (Menggunakan analogReadMilliVolts agar hasil pembacaan ADC ESP32 lebih presisi)
    currentVoltage = analogReadMilliVolts(PH_PIN);
    
    // Jika ingin tetap menggunakan formula manual bawaan Anda:
    // currentVoltage = (analogRead(PH_PIN) / 4095.0) * 3300.0;

    // 2. Hitung nilai pH berdasarkan kompensasi suhu
    return ph.readPH(currentVoltage, temperature);
}

void calibratePH(float temperature) {
    // Memproses perintah kalibrasi dari Serial Monitor (enterph, calph, exitph)
    ph.calibration(currentVoltage, temperature);
}