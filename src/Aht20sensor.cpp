#include "Aht20sensor.h"

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <math.h>

// Instance sensor AHT20
static Adafruit_AHTX0 aht20;

// Status sensor
static bool sensorReady = false;

// Cache hasil pembacaan terakhir
static float lastTemperature = NAN;
static float lastHumidity = NAN;


// ==========================================
// INISIALISASI SENSOR
// ==========================================

bool aht20Begin()
{
    sensorReady = false;

    lastTemperature = NAN;
    lastHumidity = NAN;

    // Inisialisasi komunikasi I2C
    Wire.begin(AHT20_SDA_PIN, AHT20_SCL_PIN);

    // Inisialisasi sensor AHT20
    if (!aht20.begin(&Wire, -1, 0x38))
    {
        Serial.println("[AHT20] Sensor tidak terdeteksi!");

        return false;
    }

    sensorReady = true;

    Serial.println("[AHT20] Sensor berhasil diinisialisasi.");
    Serial.println("[AHT20] Alamat I2C: 0x38");

    return true;
}


// ==========================================
// MEMBACA SUHU DAN KELEMBAPAN
// ==========================================

bool aht20Read(float &temperature, float &humidity)
{
    if (!sensorReady)
    {
        Serial.println("[AHT20] Sensor belum diinisialisasi.");

        return false;
    }

    sensors_event_t humidityEvent;
    sensors_event_t temperatureEvent;

    // Membaca data sensor
    aht20.getEvent(&humidityEvent, &temperatureEvent);

    float temp = temperatureEvent.temperature;
    float hum = humidityEvent.relative_humidity;

    // Validasi data
    if (!isfinite(temp) || !isfinite(hum))
    {
        Serial.println("[AHT20] Data pembacaan tidak valid.");

        return false;
    }

    // Simpan hasil pembacaan
    lastTemperature = temp;
    lastHumidity = hum;

    temperature = temp;
    humidity = hum;

    return true;
}


// ==========================================
// MEMBACA SUHU SAJA
// ==========================================

float aht20GetTemperature()
{
    float temperature;
    float humidity;

    if (!aht20Read(temperature, humidity))
    {
        return NAN;
    }

    return temperature;
}


// ==========================================
// MEMBACA KELEMBAPAN SAJA
// ==========================================

float aht20GetHumidity()
{
    float temperature;
    float humidity;

    if (!aht20Read(temperature, humidity))
    {
        return NAN;
    }

    return humidity;
}


// ==========================================
// STATUS SENSOR
// ==========================================

bool aht20IsReady()
{
    return sensorReady;
}
