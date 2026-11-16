#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

// GPIO mapping untuk lima pompa (active‑high)
#define PUMP_A_PIN       26
#define PUMP_B_PIN       27
#define PUMP_UP_PIN      32
#define PUMP_DOWN_PIN    33
#define PUMP_BIG_PIN     25

void initPump();

// Pompa utama (big)
void pumpBigOn();
void pumpBigOff();
bool getPumpBigStatus();

// Pompa dosing A & B
void pumpAOn();
void pumpAOff();
bool getPumpAStatus();

void pumpBOn();
void pumpBOff();
bool getPumpBStatus();

// Pompa penyesuaian pH
void pumpUpOn();
void pumpUpOff();
bool getPumpUpStatus();

void pumpDownOn();
void pumpDownOff();
bool getPumpDownStatus();

#endif