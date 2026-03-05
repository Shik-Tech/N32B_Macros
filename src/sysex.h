/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#pragma once

#include <Arduino.h>
#include <EEPROM.h>

#include "definitions.h"
#include "storage.h"

/** Parses SysEx messages and dispatches to command handlers. */
void processSysex(unsigned char *data, unsigned int size);

/** Sets the global MIDI channel (1–16). */
void handleChangeChannel(byte channel);

/** Loads preset on program change. Ignores channel. */
void handleProgramChange(byte channel, byte number);

/** Sends firmware version (3 bytes) from EEPROM via USB and TRS. */
void sendDeviceFirmwareVersion();

/** Sends full active preset and thru mode, then END_OF_TRANSMISSION. */
void sendActivePreset();

/** Sets MIDI thru routing mode. Valid values 0–5 (THRU_MODES). */
void setMidiThruMode(byte mode);