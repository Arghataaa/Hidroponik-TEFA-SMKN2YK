#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Tiga Topik Utama MQTT
extern const char* topicSensor;
extern const char* topicControl;
extern const char* topicStatus;

extern PubSubClient mqtt;

// Konfigurasi interval sensor (dalam satuan MENIT)
extern unsigned long sensor_interval_minutes;

void initMQTT();
void loopMQTT();

// Publish telemetry sensor
void publishSensorData(
    float lux,
    float temperatureAir,
    float humidity,
    float ec,
    float ph,
    float temperatureWater
);

void publishSensorDataJSON(
    float lux,
    float temperatureAir,
    float humidity,
    float ec,
    float ph,
    float temperatureWater,
    float rssi
);

// Publish feedback status aktual 5 pompa dan interval aktif
void publishStatus(const char* message = "Command executed successfully");

#endif
