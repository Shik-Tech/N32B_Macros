/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "definitions.h"

USBMIDI_CREATE_INSTANCE(0, MIDICoreUSB);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDICoreSerial);

N32B_DISPLAY n32b_display(DIN, CS, CLK);

ezButton buttonA(BUTTON_A_PIN);
ezButton buttonB(BUTTON_B_PIN);

/* Device setup data */
Device_t device;

/* Buttons variables */
const unsigned int reset_timeout = 4000; // Reset to factory preset timeout
const uint8_t SHORT_PRESS_TIME = 255;    // Milliseconds
unsigned long pressedTime = 0;
bool isPressingAButton = false;
bool isPressingBButton = false;

// byte index in EEPROM for the last used preset
uint8_t lastUsedPresetAddress = 0;
