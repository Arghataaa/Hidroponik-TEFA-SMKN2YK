#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>

void initMQTT();
void loopMQTT();

void publishSensorData(
    float lux,
    float temperature,
    float humidity,
    float ph,
    float tds,
    float waterLevel
);

void setPump(bool status);

#endif