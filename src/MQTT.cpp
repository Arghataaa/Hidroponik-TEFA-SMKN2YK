#include <Arduino.h>
#include <stdio.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "MQTT.h"
#include "Pompa.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

// Definisi 2 Topic MQTT Utama
const char* topicSensor   = "tefa/hidroponik/sensor";
const char* topicAktuator = "tefa/hidroponik/aktuator";

// State Sistem (Mode & Konfigurasi)
static String systemMode = "manual";
unsigned long publishInterval = 60000; // Default 1 menit (ms)

// Callback MQTT untuk menerima perintah dari aplikasi/web pada topic aktuator
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (length == 0 || length > 512) return;

    char msgBuffer[512];
    memcpy(msgBuffer, payload, length);
    msgBuffer[length] = '\0';

    Serial.print("Pesan diterima pada topic [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(msgBuffer);

    String currentTopic = String(topic);
    if (currentTopic != topicAktuator) return;

    // Coba parse sebagai JSON terlebih dahulu
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, msgBuffer);

    if (!error) {
        // Berhasil parsing JSON
        if (doc.containsKey("mode")) {
            const char* modeStr = doc["mode"];
            if (modeStr != nullptr) {
                systemMode = String(modeStr);
                Serial.print("Mode diubah ke: ");
                Serial.println(systemMode);
            }
        }

        // Pengaturan Interval Publish (Detik atau Menit) dari platform
        if (doc.containsKey("interval_detik")) {
            publishInterval = (unsigned long)doc["interval_detik"] * 1000;
            Serial.print("Interval kirim diubah ke (detik): ");
            Serial.println(doc["interval_detik"].as<int>());
        } else if (doc.containsKey("interval_menit")) {
            publishInterval = (unsigned long)doc["interval_menit"] * 60000;
            Serial.print("Interval kirim diubah ke (menit): ");
            Serial.println(doc["interval_menit"].as<int>());
        }

        if (doc.containsKey("pump")) {
            if (systemMode == "manual") {
                if (doc["pump"].is<bool>()) {
                    bool pumpCmd = doc["pump"];
                    if (pumpCmd) pumpOn(); else pumpOff();
                } else if (doc["pump"].is<int>()) {
                    int pumpCmd = doc["pump"];
                    if (pumpCmd != 0) pumpOn(); else pumpOff();
                } else if (doc["pump"].is<const char*>()) {
                    String pStr = String(doc["pump"].as<const char*>());
                    pStr.toLowerCase();
                    if (pStr == "on" || pStr == "true" || pStr == "1" || pStr == "high") {
                        pumpOn();
                    } else if (pStr == "off" || pStr == "false" || pStr == "0" || pStr == "low") {
                        pumpOff();
                    }
                }
            } else {
                Serial.println("Perintah pompa diabaikan karena mode saat ini automatic.");
            }
        }
    } else {
        // Bukan JSON, anggap sebagai Plain Text (misal: "ON", "OFF", "true", dll)
        String rawMsg = String(msgBuffer);
        rawMsg.trim();
        rawMsg.toLowerCase();

        Serial.print("Menerima plain text: ");
        Serial.println(rawMsg);

        if (systemMode == "manual") {
            if (rawMsg == "on" || rawMsg == "true" || rawMsg == "1" || rawMsg == "high") {
                pumpOn();
                Serial.println("Pompa dinyalakan via plain text.");
            } else if (rawMsg == "off" || rawMsg == "false" || rawMsg == "0" || rawMsg == "low") {
                pumpOff();
                Serial.println("Pompa dimatikan via plain text.");
            } else {
                Serial.println("Perintah plain text tidak dikenal.");
            }
        } else {
            // Jika pesan berupa perintah ubah mode via plain text
            if (rawMsg == "manual" || rawMsg == "automatic" || rawMsg == "auto") {
                systemMode = (rawMsg == "auto") ? "automatic" : rawMsg;
                Serial.print("Mode diubah via plain text ke: ");
                Serial.println(systemMode);
            } else {
                Serial.println("Perintah pompa diabaikan karena mode saat ini automatic.");
            }
        }
    }

    // Kirim balik status aktual aktuator ke topic yang sama
    publishAktuatorStatus();
}

void initMQTT() {
    mqtt.setBufferSize(1024); // Perbesar batas paket MQTT dari 256 ke 1024 byte
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);

    String clientId = "ESP32-" + String(random(0xffff), HEX);
    
    if (mqtt.connect(clientId.c_str())) {
        Serial.println("MQTT Connected");
        // Subscribe ke topik aktuator untuk menerima perintah
        mqtt.subscribe(topicAktuator);
        
        // Publish status awal aktuator saat terhubung
        publishAktuatorStatus();
    } else {
        Serial.print("MQTT Fail, rc=");
        Serial.println(mqtt.state());
    }
}

void loopMQTT() {
    if (!mqtt.connected()) {
        static unsigned long lastReconnectAttempt = 0;
        unsigned long now = millis();
        // Reconnect non-blocking tiap 5 detik
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            Serial.println("Mencoba reconnect MQTT...");
            String clientId = "ESP32-" + String(random(0xffff), HEX);
            if (mqtt.connect(clientId.c_str())) {
                Serial.println("MQTT Reconnected");
                mqtt.subscribe(topicAktuator);
                publishAktuatorStatus();
            }
        }
    } else {
        mqtt.loop();
    }
}

void publishSensorDataJSON(
    float lux,
    float temperatureAir,
    float humidity,
    float ec,
    float ph,
    float temperatureWater,
    float rssi
) {
    if (!mqtt.connected()) return;

    StaticJsonDocument<512> doc;
    doc["device_id"] = "DEV-TSK-01";
    doc["timestamp"] = "2026-11-12T00:00:00Z";
    doc["wifi_rssi"] = rssi;
    doc["ph"] = serialized(String(ph, 2));
    doc["ec"] = serialized(String(ec, 2));
    doc["lux"] = serialized(String(lux, 2));
    doc["temp_udara"] = serialized(String(temperatureAir, 2));
    doc["hum_udara"] = serialized(String(humidity, 2));
    doc["temp_air"] = serialized(String(temperatureWater, 2));

    char payload[512];
    serializeJson(doc, payload);

    if (mqtt.publish(topicSensor, payload)) {
        Serial.print("Data sensor terkirim (Simple Format): ");
        Serial.println(payload);
    } else {
        Serial.println("GAGAL mengirim data sensor!");
    }
}

// Keep old for compatibility or just remove if not used
void publishSensorData(
    float lux,
    float temperatureAir,
    float humidity,
    float ec,
    float ph,
    float temperatureWater
) {
    publishSensorDataJSON(lux, temperatureAir, humidity, ec, ph, temperatureWater, -50.0);
}

void publishAktuatorStatus() {
    if (!mqtt.connected()) return;

    StaticJsonDocument<256> doc;
    doc["pump"] = getPumpStatus();
    doc["mode"] = systemMode;
    doc["interval_detik"] = publishInterval / 1000;
    doc["device"] = "ESP32";

    char payload[256];
    serializeJson(doc, payload);

    if (mqtt.publish(topicAktuator, payload)) {
        Serial.print("Status aktuator terkirim: ");
        Serial.println(payload);
    }
}

void setPump(bool status) {
    if (status) {
        pumpOn();
    } else {
        pumpOff();
    }
    publishAktuatorStatus();
}