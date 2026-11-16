#include "ECsensor.h"
#include "config.h"

ECSensor::ECSensor(uint8_t pin)
  : _pin(pin), _voltage(0), _ecValue(0), _temperature(25.0) {}

void ECSensor::begin() {
  analogReadResolution(12);
  analogSetPinAttenuation(_pin, ADC_11db);

  EEPROM.begin(EC_EEPROM_SIZE);
  _ec.begin();
}

float ECSensor::read(float temperature) {
  _temperature = temperature;

  // analogReadMilliVolts() menghasilkan tegangan dalam mV.
  _voltage = analogReadMilliVolts(_pin);

  // RES2 = 820 dan ECREF = 200, sehingga pembagi = 164.
  const float rawEC = _voltage / 164.0;
  const float kValue = (rawEC > 2.5) ? EC_KVALUE_HIGH : EC_KVALUE_LOW;

  float ecValue = rawEC * kValue;

  // Kompensasi suhu ke standar 25.0 °C.
  ecValue = ecValue / (1.0 + 0.0185 * (temperature - 25.0));

  if (ecValue < 0.0) ecValue = 0.0;
  _ecValue = ecValue;
  return _ecValue;
}

void ECSensor::calibrate() {
  // Membaca perintah 'enterec', 'calec', 'exitec' dari Serial Monitor.
  _ec.calibration(_voltage, _temperature);
}

void ECSensor::printCalibrationHelp() {
  Serial.println("=========================================");
  Serial.println("  Sistem Pembacaan & Kalibrasi Sensor EC  ");
  Serial.println("=========================================");
  Serial.println("Perintah Kalibrasi Serial Monitor:");
  Serial.println(" 1. Ketik 'enterec' -> Masuk mode kalibrasi");
  Serial.println(" 2. Ketik 'calec'   -> Proses kalkulasi titik kalibrasi");
  Serial.println(" 3. Ketik 'exitec'  -> Simpan & keluar mode kalibrasi");
  Serial.println("=========================================\n");
}

float ECSensor::getVoltage() const     { return _voltage; }
float ECSensor::getEC() const          { return _ecValue; }
float ECSensor::getTemperature() const { return _temperature; }
