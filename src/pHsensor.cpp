#include "pHsensor.h"
#include "config.h"
#include "DFRobot_PH.h"
#include <EEPROM.h>

// Variabel internal modul pH
static DFRobot_PH ph;
static float currentVoltage = 0.0;

void initPH() {
    analogReadResolution(12);
    analogSetPinAttenuation(PH_PIN, ADC_11db);
    ph.begin();
}

float readPH(float temperature) {
    (void)temperature; // Rumus kalibrasi ini tidak memerlukan kompensasi suhu.

    // analogReadMilliVolts() menghasilkan tegangan dalam mV.
    currentVoltage = analogReadMilliVolts(PH_PIN);
    const float voltage = currentVoltage;

    const float neutral = (PH_NEUTRAL_VOLTAGE - 1500.0) / 3.0;
    const float acid = (PH_ACID_VOLTAGE - 1500.0) / 3.0;
    const float slope = (7.0 - 4.0) / (neutral - acid);
    const float intercept = 7.0 - slope * neutral;

    float phValue = slope * (voltage - 1500.0) / 3.0 + intercept;

    // Batas fisik pH agar probe kering/noise tidak menghasilkan nilai invalid.
    if (phValue < 0.0) phValue = 0.0;
    if (phValue > 14.0) phValue = 14.0;
    return phValue;
}

void calibratePH(float temperature) {
    // Kompatibilitas prosedur kalibrasi Serial Monitor lama.
    ph.calibration(currentVoltage, temperature);
}
