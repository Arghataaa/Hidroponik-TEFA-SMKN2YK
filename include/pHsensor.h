#ifndef PHSENSOR_H
#define PHSENSOR_H

#include <Arduino.h>

// Deklarasi fungsi utama sensor pH
void initPH();
float readPH(float temperature);
void calibratePH(float temperature); // Untuk menangani perintah Serial (enterph, calph, exitph)

#endif