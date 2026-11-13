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

#define SLEEP_DURATION_SEC 10

ECSensor ecSensor;
WiFiManagerModule wifiModule;
KekuatanSinyal wifiSignal;
extern unsigned long publishInterval;

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

    // Init I2C & Hardware
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    scanI2C();
    initLCD();
    initPump();
    initSuhuAir();
    initPH();
    ecSensor.begin();
    aht20Begin();
    initBH1750();

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

    // Baca Sensor (Ganti useDummy ke false jika menggunakan sensor fisik asli)
    bool useDummy = true; 
    
    float suhuAir = useDummy ? 26.80 : getTemp();
    float nilaiPH = useDummy ? 7.20 : readPH(suhuAir);
    float nilaiEC = useDummy ? 1.45 : ecSensor.read(suhuAir);
    
    float suhuUdara = 0.0, kelembapan = 0.0;
    if (useDummy) {
        suhuUdara = 28.50;
        kelembapan = 65.20;
    } else {
        aht20Read(suhuUdara, kelembapan);
    }
    
    float lux = useDummy ? 450.50 : readBH1750();
    int rssi = wifiSignal.getRSSI(); 

    // Tampilkan semua data sensor ke LCD 20x4 secara stabil
    tampilkanSensor(suhuAir, nilaiPH, nilaiEC, suhuUdara, kelembapan);

    // Print Log Serial
    Serial.printf("Temp Air: %.2f C | pH: %.2f | EC: %.2f mS/cm\n", suhuAir, nilaiPH, nilaiEC);
    Serial.printf("Temp Udara: %.2f C | Hum: %.2f %% | Lux: %.2f\n", suhuUdara, kelembapan, lux);
    Serial.printf("Signal: %s | Pump: %s\n", getWiFiSignalQuality().c_str(), getPumpStatus() ? "ON" : "OFF");

    // Publish Data Sensor berkala sesuai interval dari platform
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > publishInterval) {
        lastPublish = millis();
        if (wifiModule.isConnected()) {
            publishSensorDataJSON(lux, suhuUdara, kelembapan, nilaiEC, nilaiPH, suhuAir, (float)rssi);
        }
    }

    delay(10000); // Interval pembaruan tampilan dan pembacaan
}