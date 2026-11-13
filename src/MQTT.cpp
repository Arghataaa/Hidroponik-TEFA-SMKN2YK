
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

// Topic MQTT
const char* topicSensor   = "tefa/hidroponik/sensor";
const char* topicAktuator = "tefa/hidroponik/aktuator";

// State sistem
static String systemMode = "manual";

// Default interval pengiriman 10 detik
unsigned long publishInterval = 10000;

// Callback MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (length == 0 || length > 512) return;

    char msgBuffer[513];
    memcpy(msgBuffer, payload, length);
    msgBuffer[length] = '\0';

    Serial.print("Pesan diterima [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(msgBuffer);

    if (String(topic) != topicAktuator) return;

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, msgBuffer);

    if (!error) {

        // Pengaturan mode
        if (doc.containsKey("mode")) {
            const char* modeStr = doc["mode"];

            if (modeStr != nullptr) {
                String newMode = String(modeStr);
                newMode.toLowerCase();

                if (newMode == "manual" || newMode == "automatic") {
                    systemMode = newMode;
                    Serial.println("Mode diubah: " + systemMode);
                }
            }
        }

        // Pengaturan interval dalam detik
        if (doc.containsKey("interval_detik")) {
            unsigned long seconds = doc["interval_detik"];

            if (seconds >= 1 && seconds <= 3600) {
                publishInterval = seconds * 1000UL;
                Serial.print("Interval diubah: ");
                Serial.print(seconds);
                Serial.println(" detik");
            }
        }
        // Pengaturan interval dalam menit
        else if (doc.containsKey("interval_menit")) {
            unsigned long minutes = doc["interval_menit"];

            if (minutes >= 1 && minutes <= 60) {
                publishInterval = minutes * 60000UL;
                Serial.print("Interval diubah: ");
                Serial.print(minutes);
                Serial.println(" menit");
            }
        }

        // Kontrol pompa mode manual
        if (doc.containsKey("pump")) {
            if (systemMode == "manual") {

                if (doc["pump"].is<bool>()) {
                    bool pumpCmd = doc["pump"];
                    if (pumpCmd) pumpOn();
                    else pumpOff();
                }
                else if (doc["pump"].is<int>()) {
                    int pumpCmd = doc["pump"];
                    if (pumpCmd != 0) pumpOn();
                    else pumpOff();
                }
                else if (doc["pump"].is<const char*>()) {
                    String pStr = doc["pump"].as<String>();
                    pStr.toLowerCase();

                    if (pStr == "on" || pStr == "true" ||
                        pStr == "1" || pStr == "high") {
                        pumpOn();
                    }
                    else if (pStr == "off" || pStr == "false" ||
                             pStr == "0" || pStr == "low") {
                        pumpOff();
                    }
                }
            }
            else {
                Serial.println("Perintah pompa diabaikan: mode automatic.");
            }
        }
    }
    else {
        // Plain text
        String rawMsg = String(msgBuffer);
        rawMsg.trim();
        rawMsg.toLowerCase();

        if (rawMsg == "manual" || rawMsg == "automatic" || rawMsg == "auto") {
            systemMode = (rawMsg == "auto") ? "automatic" : rawMsg;
            Serial.println("Mode: " + systemMode);
        }
        else if (systemMode == "manual") {
            if (rawMsg == "on" || rawMsg == "true" ||
                rawMsg == "1" || rawMsg == "high") {
                pumpOn();
            }
            else if (rawMsg == "off" || rawMsg == "false" ||
                     rawMsg == "0" || rawMsg == "low") {
                pumpOff();
            }
        }
    }

    publishAktuatorStatus();
}

// Inisialisasi MQTT
void initMQTT() {
    mqtt.setBufferSize(1024);
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);

    String clientId = "ESP32-" + String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str())) {
        Serial.println("MQTT Connected");

        mqtt.subscribe(topicAktuator);
        publishAktuatorStatus();
    }
    else {
        Serial.print("MQTT Fail, rc=");
        Serial.println(mqtt.state());
    }
}

// Loop MQTT dan reconnect
void loopMQTT() {
    if (!mqtt.connected()) {
        static unsigned long lastReconnectAttempt = 0;
        unsigned long now = millis();

        if (now - lastReconnectAttempt >= 5000) {
            lastReconnectAttempt = now;

            Serial.println("Mencoba reconnect MQTT...");

            String clientId = "ESP32-" + String(random(0xffff), HEX);

            if (mqtt.connect(clientId.c_str())) {
                Serial.println("MQTT Reconnected");

                mqtt.subscribe(topicAktuator);
                publishAktuatorStatus();
            }
        }
    }
    else {
        mqtt.loop();
    }
}

// Publish data sensor JSON
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
        Serial.print("Data sensor terkirim: ");
        Serial.println(payload);
    }
    else {
        Serial.println("GAGAL mengirim data sensor!");
    }
}

// Kompatibilitas fungsi lama
void publishSensorData(
    float lux,
    float temperatureAir,
    float humidity,
    float ec,
    float ph,
    float temperatureWater
) {
    publishSensorDataJSON(
        lux,
        temperatureAir,
        humidity,
        ec,
        ph,
        temperatureWater,
        -50.0
    );
}

// Publish status aktuator
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

// Kontrol pompa
void setPump(bool status) {
    if (status) {
        pumpOn();
    }
    else {
        pumpOff();
    }

    publishAktuatorStatus();
}
