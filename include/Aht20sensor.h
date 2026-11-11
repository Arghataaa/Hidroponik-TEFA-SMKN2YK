#ifndef AHT20SENSOR_H
#define AHT20SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>

// Deklarasi fungsi sensor AHT20
bool initAHT20();
float readAHT20Temperature();
float readAHT20Humidity();

#endif
