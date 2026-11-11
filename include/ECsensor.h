#ifndef ECSENSOR_H
#define ECSENSOR_H

#include <Arduino.h>
#include <EEPROM.h>
#include "DFRobot_EC.h"

#ifndef EC_PIN
#define EC_PIN 35
#endif

#define EC_EEPROM_SIZE 32

class ECSensor {
public:
  ECSensor(uint8_t pin = EC_PIN);

  void  begin();
  float read(float temperature = 25.0);   // baca tegangan lalu hitung EC (mS/cm)
  void  calibrate();                      // panggil terus-menerus di loop()
  void  printCalibrationHelp();

  float getVoltage() const;               // mV
  float getEC() const;                    // mS/cm
  float getTemperature() const;           // °C

private:
  uint8_t   _pin;
  float     _voltage;
  float     _ecValue;
  float     _temperature;
  DFRobot_EC _ec;
};

#endif