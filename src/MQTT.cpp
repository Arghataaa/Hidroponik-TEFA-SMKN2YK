#include <Arduino.h>
#include <stdio.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"
#include "MQTT.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* topicSensor = "tefa/hidroponik/sensor";

void initMQTT() {
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
        Serial.println("MQTT Connected");
    } else {
        Serial.print("MQTT Fail, rc=");
        Serial.println(mqtt.state());
    }
}

void loopMQTT() {
    if (!mqtt.connected()) {
        initMQTT();
    }
    mqtt.loop();
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

    char payload[512];
    snprintf(
        payload,
        sizeof(payload),
        "{"
        "\"lux\":%.2f,"
        "\"temperature_air\":%.2f,"
        "\"humidity\":%.2f,"
        "\"ec\":%.2f,"
        "\"ph\":%.2f,"
        "\"temperature_water\":%.2f"
        "}",
        lux,
        temperatureAir,
        humidity,
        ec,
        ph,
        temperatureWater
    );

    if (mqtt.publish(topicSensor, payload)) {
        Serial.print("Data sensor terkirim: ");
        Serial.println(payload);
    }
}

void setPump(bool status) {
    // Fungsi opsional untuk kontrol pompa via MQTT
}