/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#ifndef SYSEX_h
#define SYSEX_h

#include <Arduino.h>
#include <EEPROM.h>

#include "definitions.h"
#include "storage.h"
#include "functions.h"

void processSysex(unsigned char *, unsigned int);
void handleChangeChannel(byte);
void handleProgramChange(byte, byte);
void sendDeviceFirmwareVersion();
void sendActivePreset();
void setMidiThruMode(byte);

#endif