#include "bluetooth.h"
#include "BluetoothSerial.h"

// Membuat objek Bluetooth
BluetoothSerial SerialBT;

// Inisialisasi Bluetooth
bool initBluetooth() {

    if (SerialBT.begin("MAPPI32-Hydroponic")) {

        Serial.println("Bluetooth berhasil diaktifkan");
        Serial.println("Nama: MAPPI32-Hydroponic");

        return true;

    } else {

        Serial.println("Bluetooth gagal diaktifkan");

        return false;
    }
}

// Mengirim pesan ke HP
void sendBluetooth(String message) {

    if (SerialBT.hasClient()) {
        SerialBT.println(message);
    }
}

// Menerima pesan dari HP
String readBluetooth() {

    static String incomingBuffer = "";

    while (SerialBT.available()) {

        char incomingChar = SerialBT.read();

        if (incomingChar == '\n') {

            incomingBuffer.trim();

            String message = incomingBuffer;
            incomingBuffer = "";

            return message;
        }

        if (incomingChar != '\r') {
            incomingBuffer += incomingChar;
        }
    }

    return "";
}