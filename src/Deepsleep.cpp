#include "Deepsleep.h"

void startDeepSleep(uint64_t seconds) {
    Serial.print("ESP32 memasuki mode Deep Sleep selama ");
    Serial.print(seconds);
    Serial.println(" detik...");
    
    // Pastikan seluruh teks di Serial Monitor selesai terkirim sebelum tidur
    Serial.flush(); 

    // Atur timer bangun (Konversi detik ke mikrodetik: 1 detik = 1.000.000 µs)
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
    
    // Matikan ESP32 ke mode Deep Sleep
    esp_deep_sleep_start();
}