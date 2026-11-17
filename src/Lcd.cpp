#include "Lcd.h"
#include "config.h"

#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

static bool waktuTersinkron = false;
static const int DAYLIGHT_OFFSET_SEC = 0;

// Menulis tepat 20 karakter agar sisa karakter tampilan sebelumnya tidak tertinggal.
static void tulisBarisLCD(uint8_t row, const char* text) {
    char line[LCD_COLS + 1];
    snprintf(line, sizeof(line), "%-20.20s", text);
    lcd.setCursor(0, row);
    lcd.print(line);
}

void initLCD() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    tulisBarisLCD(0, "HIDROPONIK TEFA");
    tulisBarisLCD(1, "SMKN 2 YK");
    tulisBarisLCD(3, "Starting...");
}

void tampilkanStatus(String baris1, String baris2) {
    lcd.clear();
    tulisBarisLCD(1, baris1.c_str());
    tulisBarisLCD(2, baris2.c_str());
}

bool sinkronisasiWaktuWIB() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[NTP] WiFi belum terhubung; sinkronisasi WIB dilewati.");
        waktuTersinkron = false;
        return false;
    }

    Serial.println("[NTP] Memulai sinkronisasi waktu WIB (GMT+7)...");
    configTime(WIB_UTC_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER_PRIMARY, NTP_SERVER_SECONDARY);

    struct tm timeInfo;
    if (!getLocalTime(&timeInfo, 10000)) {
        Serial.println("[NTP] Sinkronisasi waktu gagal (timeout). ");
        waktuTersinkron = false;
        return false;
    }

    char waktu[22];
    strftime(waktu, sizeof(waktu), "%d/%m/%Y %H:%M:%S", &timeInfo);
    Serial.print("[NTP] Waktu WIB tersinkron: ");
    Serial.println(waktu);
    waktuTersinkron = true;
    return true;
}

void tampilkanSensor(float suhuAir, float ph, float ec, float suhuUdara,
                     float kelembapan, float lux) {
    // Derajat LCD HD44780 adalah karakter 0xDF, bukan simbol UTF-8 °.
    const char degree = static_cast<char>(0xDF);
    char baris1[21];
    char baris2[21];
    char baris3[21];
    char baris4[21];

    // Semua format dibatasi maksimal 20 karakter untuk LCD 20x4.
    snprintf(baris1, sizeof(baris1), "pH :%5.2f  EC :%4.2f", ph, ec);
    snprintf(baris2, sizeof(baris2), "Air:%4.1f%cC Lux:%5.0f", suhuAir, degree, lux);
    snprintf(baris3, sizeof(baris3), "Udr:%4.1f%cC RH :%4.1f%%", suhuUdara, degree, kelembapan);

    struct tm timeInfo;
    if (waktuTersinkron && getLocalTime(&timeInfo, 10)) {
        strftime(baris4, sizeof(baris4), "%d/%m/%Y %H:%M:%S", &timeInfo);
    } else {
        snprintf(baris4, sizeof(baris4), "Waktu belum sinkron");
    }

    tulisBarisLCD(0, baris1);
    tulisBarisLCD(1, baris2);
    tulisBarisLCD(2, baris3);
    tulisBarisLCD(3, baris4);
}
