#include "Lcd.h"
#include "config.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

void initLCD() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("HIDROPONIK TEFA");
    lcd.setCursor(5, 1);
    lcd.print("SMKN 2 YK");
    lcd.setCursor(4, 3);
    lcd.print("Starting...");
}

void tampilkanStatus(String baris1, String baris2) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(baris1);
    lcd.setCursor(0, 2);
    lcd.print(baris2);
}

void tampilkanSensor(float suhuAir, float ph, float ec, float suhuUdara, float kelembapan) {
    lcd.clear();

    // Baris 0: Judul
    lcd.setCursor(0, 0);
    lcd.print("== MONITOR SENSOR ==");

    // Baris 1: pH dan EC
    lcd.setCursor(0, 1);
    lcd.print("pH: ");
    lcd.print(ph, 1);
    lcd.setCursor(10, 1);
    lcd.print("EC: ");
    lcd.print(ec, 1);

    // Baris 2: Suhu Air
    lcd.setCursor(0, 2);
    lcd.print("Suhu Air : ");
    lcd.print(suhuAir, 1);
    lcd.print(" C");

    // Baris 3: Suhu Udara & Kelembapan
    lcd.setCursor(0, 3);
    lcd.print("Udr:");
    lcd.print(suhuUdara, 1);
    lcd.print("C");
    lcd.setCursor(11, 3);
    lcd.print("RH:");
    lcd.print(kelembapan, 0);
    lcd.print("%");
}