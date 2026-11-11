#include "Aht20sensor.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>

static Adafruit_AHTX0 aht;
static bool ahtReady = false;

bool aht20Begin() {
  Wire.begin(AHT20_SDA_PIN, AHT20_SCL_PIN);

  Serial.println("Inisialisasi Sensor AHT20...");

  if (!aht.begin(&Wire)) {
    Serial.println("Gagal menemukan sensor AHT20! Periksa kembali perkabelan.");
    ahtReady = false;
    return false;
  }

  ahtReady = true;
  Serial.println("Sensor AHT20 berhasil diinisialisasi!");
  return true;
}

bool aht20Read(float &temperature, float &humidity) {
  if (!ahtReady) {
    return false;
  }

  sensors_event_t humidityEvent, tempEvent;
  if (!aht.getEvent(&humidityEvent, &tempEvent)) {
    return false;
  }

  temperature = tempEvent.temperature;
  humidity = humidityEvent.relative_humidity;
  return true;
}

float aht20GetTemperature() {
  float t, h;
  return aht20Read(t, h) ? t : NAN;
}

float aht20GetHumidity() {
  float t, h;
  return aht20Read(t, h) ? h : NAN;
}

bool aht20IsReady() {
  return ahtReady;
}