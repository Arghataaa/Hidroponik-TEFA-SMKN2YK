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
    Serial.println("System Wakeup");

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
    bool useWiFi = true; // Ubah ke false jika ingin tes tampilan LCD tanpa WiFi

    if (useWiFi) {
        tampilkanStatus("Konek WiFi...", "AP: Hidroponik-TEFA");
        if (wifiModule.begin()) {
            Serial.print("WiFi Connected: ");
            Serial.println(wifiModule.getIP());
            tampilkanStatus("WiFi Terhubung!", wifiModule.getIP());
            initMQTT();
            loopMQTT();
        } else {
            Serial.println("WiFi Failed");
            tampilkanStatus("WiFi Gagal!", "Mode Offline...");
        }
    } else {
        tampilkanStatus("Mode Offline", "Tes Data Dummy");
        delay(1000);
    }

    // Read Sensors (With Dummy Data Override)
    bool useDummy = true; // Set ke false jika ingin menggunakan sensor asli
    
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

    // Tampilkan semua data sensor ke LCD 20x4
    tampilkanSensor(suhuAir, nilaiPH, nilaiEC, suhuUdara, kelembapan);

    // Print Log
    Serial.printf("Temp Air: %.2f C | pH: %.2f | EC: %.2f mS/cm\n", suhuAir, nilaiPH, nilaiEC);
    Serial.printf("Temp Udara: %.2f C | Hum: %.2f %% | Lux: %.2f\n", suhuUdara, kelembapan, lux);
    Serial.printf("Signal: %s | Pump: %s\n", getWiFiSignalQuality().c_str(), getPumpStatus() ? "ON" : "OFF");

    // Publish Data & Sleep
    if (useWiFi) {
        // Tunggu hingga MQTT benar-benar terhubung (maksimal 5 detik)
        unsigned long startWait = millis();
        while (!mqtt.connected() && (millis() - startWait < 5000)) {
            loopMQTT();
            delay(100);
        }

        if (mqtt.connected()) {
            publishSensorDataJSON(lux, suhuUdara, kelembapan, nilaiEC, nilaiPH, suhuAir, (float)rssi);
            loopMQTT();
            delay(2000); // Beri waktu agar paket MQTT benar-benar terkirim ke broker
        } else {
            Serial.println("Gagal publish sensor: MQTT belum terhubung!");
        }
    }

    Serial.println("Entering Deep Sleep...");
    Serial.flush();
    startDeepSleep(SLEEP_DURATION_SEC);
}

void loop() {
    // Empty (handled in setup + deep sleep)
}