# AGENTS.md

## Overview
ESP32 IoT Hydroponics monitoring and control system (Smart Hydroponics TEFA SMKN 2 Yogyakarta) built with PlatformIO using the Arduino framework.

## Build & Verify Commands
- **Build firmware:** `pio run`
- **Upload firmware:** `pio run --target upload`
- **Monitor serial output:** `pio device monitor`
- **Clean build artifacts:** `pio run --target clean`

## Hardware & Pin Mapping (`include/config.h`)
- **I2C Bus (Shared AHT20 & BH1750):** SDA = GPIO 21, SCL = GPIO 22
- **OneWire Sensor (DS18B20 Water Temperature):** GPIO 13
- **Analog Sensors (ADC1 - Wi-Fi Safe):**
  - pH Sensor: GPIO 34 (`PH_PIN`)
  - EC Sensor: GPIO 35 (`EC_PIN`)
- **Display (TFT_eSPI ILI9488):** SPI pins configured via `platformio.ini` build flags (MOSI=23, MISO=19, SCLK=18, CS=14, DC=12, RST=5).

## Architecture & Quirks
- **Execution Flow:** Single-pass execution inside `setup()` in `src/main.cpp`. The system boots, initializes sensors and peripherals, connects to Wi-Fi/MQTT via `WiFiManager`, publishes sensor payloads, and enters **Deep Sleep** (`SLEEP_DURATION_SEC = 180` seconds). `loop()` is intentionally left empty.
- **Memory partitions:** Uses `huge_app.csv` partition table in `platformio.ini` to provide 3MB app space for heavy libraries (TFT, WiFiManager, MQTT).
