/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#ifndef STORAGE_h
#define STORAGE_h

#include <Arduino.h>
#include <EEPROM.h>

#include "devices.h"

bool isEEPROMvalid();
void formatFactory();
void loadPreset(uint8_t);
void savePreset(uint8_t);

#ifndef N32Bv3
const uint8_t knobsLocation[] PROGMEM = {
    15, 13, 8, 6,
    14, 11, 7, 5,
    12, 10, 4, 3,
    9, 2, 1, 0,
    31, 30, 29, 28,
    24, 25, 26, 27,
    20, 21, 22, 23,
    16, 17, 18, 19};
#endif

#endif