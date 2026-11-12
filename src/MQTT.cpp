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

    char jsonBuffer[512];
    memcpy(jsonBuffer, payload, length);
    jsonBuffer[length] = '\0';

    Serial.print("Pesan diterima pada topic [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(jsonBuffer);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonBuffer);

    if (error) {
        Serial.print("JSON parsing gagal: ");
        Serial.println(error.f_str());
        return;
    }

    String currentTopic = String(topic);

    // Mengontrol aktuator atau mode via topic aktuator
    if (currentTopic == topicAktuator) {
        if (doc.containsKey("mode")) {
            const char* modeStr = doc["mode"];
            if (modeStr != nullptr) {
                systemMode = String(modeStr);
                Serial.print("Mode diubah ke: ");
                Serial.println(systemMode);
            }
        }

        if (doc.containsKey("pump")) {
            bool pumpCmd = doc["pump"];
            if (systemMode == "manual") {
                if (pumpCmd) {
                    pumpOn();
                } else {
                    pumpOff();
                }
            } else {
                Serial.println("Perintah pompa diabaikan karena mode saat ini automatic.");
            }
        }

        // Kirim balik status aktual aktuator ke topic yang sama
        publishAktuatorStatus();
    }
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

void publishSensorData(
    float lux,
    float temperatureAir,
    float humidity,
    float ec,
    float ph,
    float temperatureWater
) {
    if (!mqtt.connected()) return;

    StaticJsonDocument<256> doc;
    doc["lux"] = lux;
    doc["temperature_air"] = temperatureAir;
    doc["humidity"] = humidity;
    doc["ec"] = ec;
    doc["ph"] = ph;
    doc["temperature_water"] = temperatureWater;

    char payload[256];
    serializeJson(doc, payload);

    if (mqtt.publish(topicSensor, payload)) {
        Serial.print("Data sensor terkirim: ");
        Serial.println(payload);
    }
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
