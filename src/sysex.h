/*
  N32B Macros Firmware v4.0.0
  MIT License

  Copyright (c) 2023 SHIK
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
void setMidiOutputMode(byte);

#endif