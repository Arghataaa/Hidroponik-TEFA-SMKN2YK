#include "Pompa.h"

// Pin kontrol relay
#define RELAY_PIN 25

// Status pompa
bool pumpStatus = false;

void initPump() {

    pinMode(RELAY_PIN, OUTPUT);

    // Kondisi awal: pompa mati
    digitalWrite(RELAY_PIN, HIGH);

    pumpStatus = false;

    Serial.println("Relay berhasil diinisialisasi");
    Serial.println("Pompa dalam kondisi OFF");
}

void pumpOn() {

    digitalWrite(RELAY_PIN, LOW);

    pumpStatus = true;

    Serial.println("Pompa ON");
}

void pumpOff() {

    digitalWrite(RELAY_PIN, HIGH);

    pumpStatus = false;

    Serial.println("Pompa OFF");
}

bool getPumpStatus() {

    return pumpStatus;
}