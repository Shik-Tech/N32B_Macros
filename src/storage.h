/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#pragma once

#include <Arduino.h>
#include <JC_EEPROM.h>

#include "devices.h"

//#define EEPROM_2048Kb

#ifdef EEPROM_2048Kb
const JC_EEPROM::eeprom_size_t EEPROM_Size = JC_EEPROM::kbits_2048;
const uint16_t EEPROM_PageSize = 256;
#else
const JC_EEPROM::eeprom_size_t EEPROM_Size = JC_EEPROM::kbits_1024;
const uint16_t EEPROM_PageSize = 256;
#endif

void storageInit();
bool isEEPROMvalid();
void formatFactory();
void loadPreset(uint8_t);
void savePreset(uint8_t);

extern JC_EEPROM EEPROM;
