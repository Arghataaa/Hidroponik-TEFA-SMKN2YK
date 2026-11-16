#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// KONFIGURASI PIN SENSOR HIDROPONIK
// Bus I2C Utama (Shared: AHT20 & BH1750)
#define I2C_SDA_PIN      21
#define I2C_SCL_PIN      22

// Sensor Suhu Air (DS18B20 / DFRobot Temp)
#define ONEWIRE_PIN      13  // GPIO 13 ESP32

// Sensor Analog (ADC1 - Safe dari bentrok WiFi)
#define PH_PIN           34  // GPIO 34 (Analog Input pH)
#define EC_PIN           35  // GPIO 35 (Analog Input EC)

// === DATA KALIBRASI SENSOR PH & EC ===
#define PH_NEUTRAL_VOLTAGE  1327.0   // Tegangan mV pada pH 7.0
#define PH_ACID_VOLTAGE     1865.0   // Tegangan mV pada pH 4.0
#define EC_KVALUE_LOW       1.01     // Faktor kalibrasi EC 1.413 mS/cm
#define EC_KVALUE_HIGH      1.05     // Faktor kalibrasi EC 12.88 mS/cm

// === BATAS BAWAH & ATAS NUTRISI HIDROPONIK ===
#define PH_BATAS_BAWAH      5.5      // Ambang minimum pH larutan
#define PH_BATAS_ATAS       6.5      // Ambang maksimum pH larutan
#define EC_BATAS_BAWAH      1.2      // Ambang minimum kepekatan nutrisi (mS/cm)
#define EC_BATAS_ATAS       2.0      // Ambang maksimum kepekatan nutrisi (mS/cm)

// KONFIGURASI MQTT BROKER
#define MQTT_SERVER      "broker.emqx.io"  // Ganti dengan IP Laptop/PC tempat Mosquitto berjalan
#define MQTT_PORT        1883

// KONFIGURASI SERIAL
#define SERIAL_BAUD      115200

#define LCD_ADDR 0x27   // Ubah ke 0x3F jika layar nyala tapi tulisan tidak muncul
#define LCD_COLS 20
#define LCD_ROWS 4

#endif // CONFIG_H