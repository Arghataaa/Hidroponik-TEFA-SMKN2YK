#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

void initPump();
void pumpOn();
void pumpOff();
bool getPumpStatus();

#endif