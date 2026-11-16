#include "Pompa.h"

// Status aktual masing-masing pompa
static bool pumpAState = false;
static bool pumpBState = false;
static bool pumpUpState = false;
static bool pumpDownState = false;
static bool pumpBigState = false;

void initPump() {
    pinMode(PUMP_A_PIN, OUTPUT);
    pinMode(PUMP_B_PIN, OUTPUT);
    pinMode(PUMP_UP_PIN, OUTPUT);
    pinMode(PUMP_DOWN_PIN, OUTPUT);
    pinMode(PUMP_BIG_PIN, OUTPUT);

    // Kondisi awal: semua pompa mati (active-high -> LOW)
    digitalWrite(PUMP_A_PIN, LOW);
    digitalWrite(PUMP_B_PIN, LOW);
    digitalWrite(PUMP_UP_PIN, LOW);
    digitalWrite(PUMP_DOWN_PIN, LOW);
    digitalWrite(PUMP_BIG_PIN, LOW);

    pumpAState = false;
    pumpBState = false;
    pumpUpState = false;
    pumpDownState = false;
    pumpBigState = false;

    Serial.println("5 Relay pompa berhasil diinisialisasi (Active-High: LOW = OFF)");
}

// Pompa Big (Utama)
void pumpBigOn() {
    digitalWrite(PUMP_BIG_PIN, HIGH);
    pumpBigState = true;
    Serial.println("Pump Big: ON");
}

void pumpBigOff() {
    digitalWrite(PUMP_BIG_PIN, LOW);
    pumpBigState = false;
    Serial.println("Pump Big: OFF");
}

bool getPumpBigStatus() {
    return pumpBigState;
}

// Pompa Dosing A
void pumpAOn() {
    digitalWrite(PUMP_A_PIN, HIGH);
    pumpAState = true;
    Serial.println("Pump A: ON");
}

void pumpAOff() {
    digitalWrite(PUMP_A_PIN, LOW);
    pumpAState = false;
    Serial.println("Pump A: OFF");
}

bool getPumpAStatus() {
    return pumpAState;
}

// Pompa Dosing B
void pumpBOn() {
    digitalWrite(PUMP_B_PIN, HIGH);
    pumpBState = true;
    Serial.println("Pump B: ON");
}

void pumpBOff() {
    digitalWrite(PUMP_B_PIN, LOW);
    pumpBState = false;
    Serial.println("Pump B: OFF");
}

bool getPumpBStatus() {
    return pumpBState;
}

// Pompa pH Up
void pumpUpOn() {
    digitalWrite(PUMP_UP_PIN, HIGH);
    pumpUpState = true;
    Serial.println("Pump Up: ON");
}

void pumpUpOff() {
    digitalWrite(PUMP_UP_PIN, LOW);
    pumpUpState = false;
    Serial.println("Pump Up: OFF");
}

bool getPumpUpStatus() {
    return pumpUpState;
}

// Pompa pH Down
void pumpDownOn() {
    digitalWrite(PUMP_DOWN_PIN, HIGH);
    pumpDownState = true;
    Serial.println("Pump Down: ON");
}

void pumpDownOff() {
    digitalWrite(PUMP_DOWN_PIN, LOW);
    pumpDownState = false;
    Serial.println("Pump Down: OFF");
}

bool getPumpDownStatus() {
    return pumpDownState;
}
