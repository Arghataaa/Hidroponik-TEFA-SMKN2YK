#include <Arduino.h>
#include <Wire.h>

#include "config.h"
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

// Tambahkan untuk keperluan JSON
#include <ArduinoJson.h>

ECSensor ecSensor;
WiFiManagerModule wifiModule;
KekuatanSinyal wifiSignal; // Tambahkan ini

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(500);
    Serial.println("System Wakeup");

    // Init I2C & Hardware
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    initPump();
    initSuhuAir();
    initPH();
    ecSensor.begin();
    aht20Begin();
    initBH1750();

    // WiFi & MQTT
    if (wifiModule.begin()) {
        Serial.print("WiFi Connected: ");
        Serial.println(wifiModule.getIP());
        initMQTT();
        loopMQTT();
    } else {
        Serial.println("WiFi Failed");
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
    
    // Gunakan fungsi RSSI asli karena WiFi biasanya harus aktif untuk MQTT
    int rssi = wifiSignal.getRSSI(); 

    // Print Log
    Serial.printf("Temp Air: %.2f C | pH: %.2f | EC: %.2f mS/cm\n", suhuAir, nilaiPH, nilaiEC);
    Serial.printf("Temp Udara: %.2f C | Hum: %.2f %% | Lux: %.2f\n", suhuUdara, kelembapan, lux);
    Serial.printf("Signal: %s | Pump: %s\n", getWiFiSignalQuality().c_str(), getPumpStatus() ? "ON" : "OFF");

    // Publish Data & Sleep
    if (wifiModule.isConnected()) {
        // Panggil fungsi baru yang akan kita buat atau modifikasi publishSensorData
        publishSensorDataJSON(lux, suhuUdara, kelembapan, nilaiEC, nilaiPH, suhuAir, (float)rssi);
        loopMQTT();
        delay(1000);
    }

    Serial.println("Entering Deep Sleep...");
    Serial.flush();
    startDeepSleep(SLEEP_DURATION_SEC);
}

void loop() {
    // Empty (handled in setup + deep sleep)
}