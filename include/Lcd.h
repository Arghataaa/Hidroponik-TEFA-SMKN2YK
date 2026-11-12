#ifndef LCD_H
#define LCD_H

// Konfigurasi LCD I2C
#define LCD_ADDR 0x27   // Ubah ke 0x3F jika layar tidak menampilkan teks
#define LCD_COLS 20     // Jumlah kolom (16 untuk LCD 16x2, 20 untuk 20x4)
#define LCD_ROWS 4      // Jumlah baris (2 untuk LCD 16x2, 4 untuk 20x4)

#include <Arduino.h>

void initLCD();
void tampilkanStatus(String baris1, String baris2);
void tampilkanSensor(float suhuAir, float ph, float ec, float suhuUdara, float kelembapan);

#endif