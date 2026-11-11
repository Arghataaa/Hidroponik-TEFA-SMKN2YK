#include "Aht20sensor.h"

// Membuat objek sensor AHTX0 (digunakan untuk AHT20/AHT10)
Adafruit_AHTX0 aht;

// Inisialisasi sensor AHT20
bool initAHT20() {
    if (!aht.begin()) {
        Serial.println("Sensor AHT20 tidak terdeteksi! Periksa koneksi kabel.");
        return false;
    }
    Serial.println("Sensor AHT20 berhasil diinisialisasi.");
    return true;
}

// Membaca suhu dari sensor AHT20
float readAHT20Temperature() {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    return temp.temperature;
}

// Membaca kelembapan (humidity) dari sensor AHT20
float readAHT20Humidity() {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    return humidity.relative_humidity;
}
