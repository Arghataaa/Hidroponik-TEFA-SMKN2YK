#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "MQTT.h"
#include "Pompa.h"

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* topicSensor = "tefa/hidroponik/sensor";
const char* topicControl = "tefa/hidroponik/control";
const char* topicStatus = "tefa/hidroponik/status";

// Interval konfigurasi publik (dalam satuan MENIT)
unsigned long sensor_interval_minutes = 1;
static unsigned long lastReconnectAttempt = 0;

// Variabel threshold kontrol otomatis
static float phMin = 5.5;
static float phMax = 6.5;
static float ecTarget = 1.5;
static int phUpDoseSec = 2;
static int phDownDoseSec = 2;

// Helper untuk parsing "on" / "off" (case-insensitive), boolean true/false, atau 1/0
static bool parsePumpValue(JsonVariant value) {
    if (value.is<bool>()) return value.as<bool>();
    if (value.is<int>()) return value.as<int>() != 0;
    String state = value.as<String>();
    state.trim();
    state.toLowerCase();
    return (state == "on" || state == "true" || state == "1" || state == "high");
}

// Publish status aktual 5 pompa dan interval aktif ke "tefa/hidroponik/status"
void publishStatus(const char* message) {
    if (!mqtt.connected()) return;

    DynamicJsonDocument doc(512);
    doc["device_id"] = "DEV-TSK-01";
    doc["status_message"] = message;
    doc["active_interval_minutes"] = sensor_interval_minutes;

    JsonObject pumps = doc.createNestedObject("pumps_status");
    pumps["pump_A"] = getPumpAStatus() ? "ON" : "OFF";
    pumps["pump_B"] = getPumpBStatus() ? "ON" : "OFF";
    pumps["pump_up"] = getPumpUpStatus() ? "ON" : "OFF";
    pumps["pump_down"] = getPumpDownStatus() ? "ON" : "OFF";
    pumps["pump_big"] = getPumpBigStatus() ? "ON" : "OFF";

    String payload;
    payload.reserve(512);
    serializeJson(doc, payload);
    if (mqtt.publish(topicStatus, payload.c_str())) {
        Serial.print("Status terkirim ke [");
        Serial.print(topicStatus);
        Serial.print("]: ");
        Serial.println(payload);
    } else {
        Serial.println("Publish status gagal.");
    }
}

// Callback subscription: menerima perintah dari topik "tefa/hidroponik/control"
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (String(topic) != topicControl || length == 0 || length > 1024) return;

    // Gunakan heap untuk payload besar agar tidak menghabiskan stack loopTask.
    String buffer;
    buffer.reserve(length + 1);
    buffer.concat(reinterpret_cast<const char*>(payload), length);

    Serial.print("Control diterima [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(buffer);

    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, buffer);
    if (err) {
        Serial.println("Control diabaikan: JSON tidak valid.");
        return;
    }

    // 1. Update interval (satuan MENIT)
    if (doc.containsKey("interval_minutes")) {
        unsigned long minutes = doc["interval_minutes"] | 0UL;
        if (minutes >= 1 && minutes <= 60) {
            sensor_interval_minutes = minutes;
            Serial.printf("Interval sensor diubah: %lu menit\n", sensor_interval_minutes);
        }
    }

    // 2. Eksekusi 5 Pompa (Active-High: HIGH = ON, LOW = OFF)
    if (doc.containsKey("pumps")) {
        JsonObject pumps = doc["pumps"];

        if (pumps.containsKey("pump_A")) {
            if (parsePumpValue(pumps["pump_A"])) pumpAOn();
            else pumpAOff();
        }
        if (pumps.containsKey("pump_B")) {
            if (parsePumpValue(pumps["pump_B"])) pumpBOn();
            else pumpBOff();
        }
        if (pumps.containsKey("pump_up")) {
            if (parsePumpValue(pumps["pump_up"])) pumpUpOn();
            else pumpUpOff();
        }
        if (pumps.containsKey("pump_down")) {
            if (parsePumpValue(pumps["pump_down"])) pumpDownOn();
            else pumpDownOff();
        }
        if (pumps.containsKey("pump_big")) {
            if (parsePumpValue(pumps["pump_big"])) pumpBigOn();
            else pumpBigOff();
        }
    }

    // 3. Update Threshold
    if (doc.containsKey("threshold")) {
        JsonObject threshold = doc["threshold"];
        if (threshold.containsKey("ph_min")) phMin = threshold["ph_min"];
        if (threshold.containsKey("ph_max")) phMax = threshold["ph_max"];
        if (threshold.containsKey("ec_target")) ecTarget = threshold["ec_target"];
        if (threshold.containsKey("ph_up_dose_sec")) phUpDoseSec = threshold["ph_up_dose_sec"];
        if (threshold.containsKey("ph_down_dose_sec")) phDownDoseSec = threshold["ph_down_dose_sec"];
        Serial.println("Threshold berhasil diperbarui.");
    }

    // 4. Langsung publish feedback status aktual ke "tefa/hidroponik/status"
    publishStatus("Command executed successfully");
}

void initMQTT() {
    mqtt.setBufferSize(1536);
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);

    String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(clientId.c_str())) {
        Serial.println("MQTT Connected");
        if (mqtt.subscribe(topicControl)) {
            Serial.println("Berhasil subscribe ke topic control");
            publishStatus("MQTT connected");
        } else {
            Serial.println("MQTT subscribe control gagal.");
        }
    } else {
        Serial.printf("MQTT Fail, rc=%d\n", mqtt.state());
    }
}

void loopMQTT() {
    if (mqtt.connected()) {
        mqtt.loop();
        return;
    }

    unsigned long now = millis();
    if (now - lastReconnectAttempt < 5000UL) return;
    lastReconnectAttempt = now;

    String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(clientId.c_str())) {
        Serial.println("MQTT Reconnected");
        if (mqtt.subscribe(topicControl)) {
            Serial.println("Berhasil resubscribe ke topic control");
            publishStatus("MQTT reconnected");
        } else {
            Serial.println("MQTT subscribe ulang control gagal.");
        }
    }
}

void publishSensorData(float lux, float temperatureAir, float humidity,
                       float ec, float ph, float temperatureWater) {
    publishSensorDataJSON(lux, temperatureAir, humidity, ec, ph, temperatureWater,
                          WiFi.RSSI());
}

void publishSensorDataJSON(float lux, float temperatureAir, float humidity,
                           float ec, float ph, float temperatureWater, float rssi) {
    if (!mqtt.connected()) return;

    DynamicJsonDocument doc(1536);
    doc["device_id"] = "DEV-TSK-01";
    doc["timestamp"] = "2026-11-09T04:15:30Z";
    JsonArray sensors = doc.createNestedArray("sensors");

    // 1. pH Larutan Nutrisi
    JsonObject s = sensors.createNestedObject();
    s["id"] = "SNS-PH-001";
    s["name"] = "pH Larutan Nutrisi";
    s["value"] = serialized(String(ph, 2));
    s["satuan"] = "pH";

    // 2. Konduktivitas Listrik (EC)
    s = sensors.createNestedObject();
    s["id"] = "SNS-EC-002";
    s["name"] = "Konduktivitas Listrik (EC)";
    s["value"] = serialized(String(ec, 2));
    s["satuan"] = "mS/cm";

    // 3. Intensitas Cahaya Lingkungan
    s = sensors.createNestedObject();
    s["id"] = "SNS-BH-003";
    s["name"] = "Intensitas Cahaya Lingkungan";
    s["value"] = serialized(String(lux, 2));
    s["satuan"] = "Lux";

    // 4. Suhu & Kelembapan Udara
    s = sensors.createNestedObject();
    s["id"] = "SNS-AHT-004";
    s["name"] = "Suhu & Kelembapan Udara";
    s["temperature"] = serialized(String(temperatureAir, 2));
    s["humidity"] = serialized(String(humidity, 2));
    s["satuan_temp"] = "°C";
    s["satuan_hum"] = "% RH";

    // 5. Suhu Air Tangki Nutrisi
    s = sensors.createNestedObject();
    s["id"] = "SNS-WT-005";
    s["name"] = "Suhu Air Tangki Nutrisi";
    s["value"] = serialized(String(temperatureWater, 2));
    s["satuan"] = "°C";

    // 6. Kekuatan Sinyal WiFi (RSSI)
    s = sensors.createNestedObject();
    s["id"] = "TEL-WIFI-001";
    s["name"] = "Kekuatan Sinyal WiFi (RSSI)";
    s["value"] = (int)rssi;
    s["satuan"] = "dBm";

    String payload;
    payload.reserve(1536);
    serializeJson(doc, payload);
    if (mqtt.publish(topicSensor, payload.c_str())) {
        Serial.print("Data sensor terkirim ke [");
        Serial.print(topicSensor);
        Serial.print("]: ");
        Serial.println(payload);
    } else {
        Serial.println("Publish sensor gagal.");
    }
}
