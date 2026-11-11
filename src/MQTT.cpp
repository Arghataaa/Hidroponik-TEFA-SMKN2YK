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
        "{\"lux\":%.2f,"
        "\"temperature_air\":%.2f,"
        "\"humidity\":%.2f,"
        "\"ec\":%.2f,"
        "\"ph\":%.2f,"
        "\"temperature_water\":%.2f}",
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