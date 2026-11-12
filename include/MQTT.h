#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

extern PubSubClient mqtt;

void initMQTT();
void loopMQTT();

void publishSensorData(
    float lux,
    float temperatureAir,
    float humidity,
    float ec,
    float ph,
    float temperatureWater
);

void setPump(bool status);

#endif