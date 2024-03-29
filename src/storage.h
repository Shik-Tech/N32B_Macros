/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#ifndef STORAGE_h
#define STORAGE_h

#include <Arduino.h>
#include <EEPROM.h>

#include "definitions.h"

bool isEEPROMvalid();
void formatFactory();
void loadPreset(uint8_t);
void savePreset(uint8_t);

#ifndef N32Bv3
const uint8_t knobsLocation[] PROGMEM = {
    15, 14, 12, 9, 31, 24, 20, 16,
    13, 11, 10, 2, 30, 25, 21, 17,
    8, 7, 4, 1, 29, 26, 22, 18,
    6, 5, 3, 0, 28, 27, 23, 19};
#endif

#endif