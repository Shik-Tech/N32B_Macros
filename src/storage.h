/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#pragma once

#include <Arduino.h>
#include <EEPROM.h>

/**
 * Returns true if EEPROM contains a valid firmware signature at the end.
 *
 * @return true if signature matches firmwareVersion, false otherwise.
 *
 * @note Returns false if EEPROM.length() < kFirmwareSignatureLen.
 */
bool isEEPROMvalid();

/**
 * Writes factory default presets to all slots and firmware signature.
 *
 * @note Call from main loop only.
 */
void formatFactory();

/**
 * Loads the specified preset into RAM and updates the active preset index.
 *
 * @param presetNumber Preset slot index (0..NUMBER_OF_PRESETS-1).
 *
 * @note Last-used preset is not persisted to EEPROM to avoid wear on byte 0.
 */
void loadPreset(uint8_t presetNumber);

/**
 * Saves the active preset to the specified EEPROM slot.
 *
 * @param presetNumber Preset slot index (0..NUMBER_OF_PRESETS-1).
 */
void savePreset(uint8_t presetNumber);

#ifndef N32Bv3
/** N32B knob index to physical mux channel mapping (non-sequential layout). */
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