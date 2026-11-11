#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <Arduino.h>

bool initBluetooth();
void sendBluetooth(String message);
String readBluetooth();

#endif