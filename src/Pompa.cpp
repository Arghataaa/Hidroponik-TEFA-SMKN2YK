#include "Pompa.h"

// Modul relay 4-channel / 2-channel pada umumnya adalah Active-LOW:
// LOW  = Relay menyala (ON / kontak NO tertutup)
// HIGH = Relay mati (OFF / kontak NO terbuka)
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// Status aktual masing-masing pompa
static bool pumpAState = false;
static bool pumpBState = false;
static bool pumpUpState = false;
static bool pumpDownState = false;
static bool pumpBigState = false;

void initPump() {
    // Set level pin ke OFF (HIGH) terlebih dahulu sebelum OUTPUT agar tidak 'click' sesaat saat boot
    digitalWrite(PUMP_A_PIN, RELAY_OFF);
    digitalWrite(PUMP_B_PIN, RELAY_OFF);
    digitalWrite(PUMP_UP_PIN, RELAY_OFF);
    digitalWrite(PUMP_DOWN_PIN, RELAY_OFF);
    digitalWrite(PUMP_BIG_PIN, RELAY_OFF);

    pinMode(PUMP_A_PIN, OUTPUT);
    pinMode(PUMP_B_PIN, OUTPUT);
    pinMode(PUMP_UP_PIN, OUTPUT);
    pinMode(PUMP_DOWN_PIN, OUTPUT);
    pinMode(PUMP_BIG_PIN, OUTPUT);

    pumpAState = false;
    pumpBState = false;
    pumpUpState = false;
    pumpDownState = false;
    pumpBigState = false;

    Serial.println("5 Relay pompa berhasil diinisialisasi (Active-Low: LOW = ON, HIGH = OFF)");
}

// Pompa Big (Utama)
void pumpBigOn() {
    digitalWrite(PUMP_BIG_PIN, RELAY_ON);
    pumpBigState = true;
    Serial.println("Pump Big: ON");
}

void pumpBigOff() {
    digitalWrite(PUMP_BIG_PIN, RELAY_OFF);
    pumpBigState = false;
    Serial.println("Pump Big: OFF");
}

bool getPumpBigStatus() {
    return pumpBigState;
}

// Pompa Dosing A
void pumpAOn() {
    digitalWrite(PUMP_A_PIN, RELAY_ON);
    pumpAState = true;
    Serial.println("Pump A: ON");
}

void pumpAOff() {
    digitalWrite(PUMP_A_PIN, RELAY_OFF);
    pumpAState = false;
    Serial.println("Pump A: OFF");
}

bool getPumpAStatus() {
    return pumpAState;
}

// Pompa Dosing B
void pumpBOn() {
    digitalWrite(PUMP_B_PIN, RELAY_ON);
    pumpBState = true;
    Serial.println("Pump B: ON");
}

void pumpBOff() {
    digitalWrite(PUMP_B_PIN, RELAY_OFF);
    pumpBState = false;
    Serial.println("Pump B: OFF");
}

bool getPumpBStatus() {
    return pumpBState;
}

// Pompa pH Up
void pumpUpOn() {
    digitalWrite(PUMP_UP_PIN, RELAY_ON);
    pumpUpState = true;
    Serial.println("Pump Up: ON");
}

void pumpUpOff() {
    digitalWrite(PUMP_UP_PIN, RELAY_OFF);
    pumpUpState = false;
    Serial.println("Pump Up: OFF");
}

bool getPumpUpStatus() {
    return pumpUpState;
}

// Pompa pH Down
void pumpDownOn() {
    digitalWrite(PUMP_DOWN_PIN, RELAY_ON);
    pumpDownState = true;
    Serial.println("Pump Down: ON");
}

void pumpDownOff() {
    digitalWrite(PUMP_DOWN_PIN, RELAY_OFF);
    pumpDownState = false;
    Serial.println("Pump Down: OFF");
}

bool getPumpDownStatus() {
    return pumpDownState;
}
