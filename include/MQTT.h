#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Topik MQTT (Dua Topik Utama: Sensor dan Aktuator)
extern const char* topicSensor;
extern const char* topicAktuator;

extern PubSubClient mqtt;

void initMQTT();
void loopMQTT();

void publishSensorDataJSON(
    float lux,
    float temperatureAir,
    float humidity,
    float ec,
    float ph,
    float temperatureWater,
    float rssi
);

void publishAktuatorStatus();
void setPump(bool status);

#endif
