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

    StaticJsonDocument<1024> doc;
    doc["device_id"] = "DEV-TSK-01"; // Sesuai Device ID di foto (DEV-TSK-01)
    doc["timestamp"] = "2026-11-12T00:00:00Z"; // Placeholder timestamp, idealnya pakai NTP

    JsonArray sensors = doc.createNestedArray("sensors");

    // WiFi
    JsonObject s1 = sensors.createNestedObject();
    s1["id"] = "TEL-WIFI-001";
    s1["value"] = rssi;
    s1["satuan"] = "dBm";

    // pH
    JsonObject s2 = sensors.createNestedObject();
    s2["id"] = "SNS-PH-001";
    s2["value"] = serialized(String(ph, 2));
    s2["satuan"] = "pH";

    // EC
    JsonObject s3 = sensors.createNestedObject();
    s3["id"] = "SNS-EC-002";
    s3["value"] = serialized(String(ec, 2));
    s3["satuan"] = "mS/cm";

    // Lux
    JsonObject s4 = sensors.createNestedObject();
    s4["id"] = "SNS-BH-003";
    s4["value"] = serialized(String(lux, 2));
    s4["satuan"] = "lux";

    // Temp Udara
    JsonObject s5 = sensors.createNestedObject();
    s5["id"] = "SNS-AHT-004_temp";
    s5["value"] = serialized(String(temperatureAir, 2));
    s5["satuan"] = "°C";

    // Hum Udara
    JsonObject s6 = sensors.createNestedObject();
    s6["id"] = "SNS-AHT-004_hum";
    s6["value"] = serialized(String(humidity, 2));
    s6["satuan"] = "%";

    // Temp Air
    JsonObject s7 = sensors.createNestedObject();
    s7["id"] = "SNS-WT-005";
    s7["value"] = serialized(String(temperatureWater, 2));
    s7["satuan"] = "°C";

    char payload[1024];
    serializeJson(doc, payload);

    if (mqtt.publish(topicSensor, payload)) {
        Serial.print("Data sensor terkirim (Format Baru): ");
        Serial.println(payload);
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
    doc["device"] = "ESP32";
    doc["connection"] = "connected";

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
