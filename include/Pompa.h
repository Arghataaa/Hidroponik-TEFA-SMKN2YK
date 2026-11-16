#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

// === RELAY 1: 4 Pompa Dosing ===
#define PUMP_A_PIN       17   // Relay 1 - CH1 (Pompa Nutrisi A)
#define PUMP_B_PIN       19   // Relay 1 - CH2 (Pompa Nutrisi B)
#define PUMP_UP_PIN      18   // Relay 1 - CH3 (Pompa pH Up)
#define PUMP_DOWN_PIN    16   // Relay 1 - CH4 (Pompa pH Down)

// === RELAY 2: Pompa Utama & Cadangan ===
#define PUMP_BIG_PIN     23   // Relay 2 - CH1 (Pompa Besar / Sirkulasi)
// #define SPARE_1_PIN   26   // Relay 2 - CH2 (Cadangan)
// #define SPARE_2_PIN   25   // Relay 2 - CH3 (Cadangan)
// Jangan pakai GPIO 12 untuk Relay 2 - CH4 (strapping pin boot ESP32)

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