#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "config.h"
#include "Lcd.h"
#include "Aht20sensor.h"
#include "Bh1750sensor.h"
#include "Suhuairsensor.h"
#include "pHsensor.h"
#include "ECsensor.h"
#include "Pompa.h"
#include "Wifimanagermodule.h"
#include "MQTT.h"
#include "Deepsleep.h"
#include "Kekuatansinyal.h"

// Deep Sleep dinonaktifkan sesuai arsitektur operasional baru.

ECSensor ecSensor;
WiFiManagerModule wifiModule;
KekuatanSinyal wifiSignal;
extern unsigned long sensor_interval_minutes;

// Mode dummy aktif hanya sebagai fallback untuk sensor yang gagal terdeteksi/dibaca.
static bool aht20Available = false;
static bool bh1750Available = false;

void scanI2C() {
    Serial.println("\n========== I2C ADDRESS SCANNER ==========");
    int count = 0;

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            count++;
            Serial.print("Alamat ditemukan: 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.print(" -> ");

            if (address == 0x23 || address == 0x5C) {
                Serial.println("BH1750");
            } else if (address == 0x38) {
                Serial.println("AHT20");
            } else if (address == 0x27 || address == 0x3F) {
                Serial.println("LCD I2C");
            } else {
                Serial.println("Unknown Device");
            }
        }
    }

    if (count == 0) {
        Serial.println("Tidak ada perangkat I2C yang ditemukan!");
    }

    Serial.print("\nTotal perangkat ditemukan: ");
    Serial.println(count);
    Serial.println("=========================================\n");
}

void setup() {
    // Mencegah ESP32 restart otomatis akibat drop tegangan saat WiFi menyala
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(SERIAL_BAUD);
    delay(500);
    Serial.println("System Booting...");

    // Init I2C & Hardware (GPIO pins preserved as configured)
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    scanI2C();
    initLCD();
    initPump();
    initSuhuAir();
    initPH();
    ecSensor.begin();
    aht20Available = aht20Begin();
    bh1750Available = initBH1750();

    // WiFi & MQTT
    bool useWiFi = true; 

    if (useWiFi) {
        tampilkanStatus("Konek WiFi...", "AP: Hidroponik-TEFA");
        if (wifiModule.begin()) {
            Serial.print("WiFi Connected: ");
            Serial.println(wifiModule.getIP());
            tampilkanStatus("WiFi Terhubung!", wifiModule.getIP());
            initMQTT();
            delay(1500);
        } else {
            Serial.println("WiFi Failed");
            tampilkanStatus("WiFi Gagal!", "Mode Offline...");
            delay(1500);
        }
    } else {
        tampilkanStatus("Mode Offline", "Tes Tanpa WiFi");
        delay(1500);
    }
}

void loop() {
    // Tangani koneksi MQTT / perintah masuk secara realtime
    loopMQTT();

    // Sensor fisik digunakan otomatis jika terdeteksi; dummy hanya fallback per sensor.
    float suhuAir = getTemp();
    if (!isfinite(suhuAir) || suhuAir < -55.0 || suhuAir > 125.0) {
        suhuAir = 26.80;
    }

    float nilaiPH = readPH(suhuAir);
    if (!isfinite(nilaiPH)) nilaiPH = 7.20;

    float nilaiEC = ecSensor.read(suhuAir);
    if (!isfinite(nilaiEC) || nilaiEC < 0.0) nilaiEC = 1.45;

    float suhuUdara = 28.50;
    float kelembapan = 65.20;
    if (aht20Available && !aht20Read(suhuUdara, kelembapan)) {
        suhuUdara = 28.50;
        kelembapan = 65.20;
    }

    float lux = bh1750Available ? readBH1750() : 450.50;
    if (!isfinite(lux) || lux < 0.0) lux = 450.50;
    wifiSignal.getRSSI();

    // Tampilkan semua data sensor ke LCD 20x4 secara stabil
    tampilkanSensor(suhuAir, nilaiPH, nilaiEC, suhuUdara, kelembapan);

    // Print log Serial dibatasi agar tidak membanjiri Serial Monitor.
    // Data sensor tetap dibaca setiap loop, tetapi log hanya dicetak tiap 5 detik.
    static unsigned long lastSerialLog = 0;
    const unsigned long serialLogIntervalMs = 5000UL;
    if (lastSerialLog == 0 || millis() - lastSerialLog >= serialLogIntervalMs) {
        lastSerialLog = millis();

        Serial.printf("Temp Air: %.2f C | pH: %.2f | EC: %.2f mS/cm\n", suhuAir, nilaiPH, nilaiEC);
        Serial.printf("Temp Udara: %.2f C | Hum: %.2f %% | Lux: %.2f\n", suhuUdara, kelembapan, lux);
        Serial.printf("Signal: %s | Pump Big: %s | A: %s | B: %s | Up: %s | Down: %s\n",
            getWiFiSignalQuality().c_str(),
            getPumpBigStatus()  ? "ON" : "OFF",
            getPumpAStatus()    ? "ON" : "OFF",
            getPumpBStatus()    ? "ON" : "OFF",
            getPumpUpStatus()   ? "ON" : "OFF",
            getPumpDownStatus() ? "ON" : "OFF");
    }

    // Publish Data Sensor berkala ke topic tefa/hidroponik/sensor
    // Penjadwalan sensor non-blocking dalam satuan menit.
    static unsigned long lastSensorPublish = 0;
    const unsigned long intervalMs = sensor_interval_minutes * 60000UL;
    if (lastSensorPublish == 0 || millis() - lastSensorPublish >= intervalMs) {
        lastSensorPublish = millis();
        if (wifiModule.isConnected()) {
            publishSensorData(lux, suhuUdara, kelembapan, nilaiEC, nilaiPH, suhuAir);
        }
    }

    // loop tetap responsif terhadap MQTT; tidak ada Deep Sleep.
    delay(10);
}
