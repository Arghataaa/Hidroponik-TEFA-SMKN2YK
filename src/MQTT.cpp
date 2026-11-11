#include "mqtt_client.h"
#include <WiFi.h>
#include <PubSubClient.h>

// Konfigurasi broker publik
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;

// Gunakan ID unik agar tidak bentrok dengan perangkat lain
const char* mqttClientID = "MAPPI32-Hydroponic-TEFA";

// Topic MQTT
const char* topicSensor = "hydroponic/tefa/sensor";
const char* topicPump = "hydroponic/tefa/pump";
const char* topicPumpControl = "hydroponic/tefa/pump/control";

WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastReconnect = 0;

// Callback menerima perintah dari broker
void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";

    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("Pesan diterima: ");
    Serial.println(message);

    if (String(topic) == topicPumpControl) {
        if (message == "ON") {
            setPump(true);
        }
        else if (message == "OFF") {
            setPump(false);
        }
    }
}

void initMQTT() {
    mqtt.setServer(mqttServer, mqttPort);
    mqtt.setCallback(callback);
    mqtt.setBufferSize(512);

    Serial.println("MQTT siap");
}

void loopMQTT() {
    if (!mqtt.connected()) {
        unsigned long now = millis();

        if (now - lastReconnect >= 5000) {
            lastReconnect = now;

            Serial.println("Menghubungkan ke broker...");

            if (mqtt.connect(mqttClientID)) {
                Serial.println("MQTT berhasil terhubung!");

                mqtt.subscribe(topicPumpControl);
            } else {
                Serial.print("MQTT gagal, kode: ");
                Serial.println(mqtt.state());
            }
        }
    }

    mqtt.loop();
}

void publishSensorData(
    float lux,
    float temperature,
    float humidity,
    float ph,
    float tds,
    float waterLevel
) {
    if (!mqtt.connected()) return;

    char payload[512];

    snprintf(
        payload,
        sizeof(payload),
        "{\"lux\":%.2f,\"temperature\":%.2f,"
        "\"humidity\":%.2f,\"ph\":%.2f,"
        "\"tds\":%.2f,\"water_level\":%.2f}",
        lux,
        temperature,
        humidity,
        ph,
        tds,
        waterLevel
    );

    if (mqtt.publish(topicSensor, payload)) {
        Serial.print("Data sensor terkirim: ");
        Serial.println(payload);
    }
}

void setPump(bool status) {
    // Fungsi ini menerima perintah aktuator.
    // Implementasi pin relay disesuaikan dengan modul yang digunakan.
    mqtt.publish(topicPump, status ? "ON" : "OFF");

    Serial.print("Perintah pompa: ");
    Serial.println(status ? "ON" : "OFF");
}