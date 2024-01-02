/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2023 SHIK
*/

#ifndef FUNCTIONS_h
#define FUNCTIONS_h

#include <Arduino.h>
#include <USB-MIDI.h>

#include "definitions.h"
#include "storage.h"

USING_NAMESPACE_MIDI;

void onUsbMessage(const midi::Message<128> &);
void onSerialMessage(const midi::Message<128> &);

void updateKnob(const uint8_t &index);

void sendCCMessage(const struct Knob_t *, uint8_t, uint8_t, midi::Channel);
void sendMacroCCMessage(const struct Knob_t *, uint8_t, uint8_t, midi::Channel, midi::Channel);
void sendRPN(const struct Knob_t *, uint8_t, uint8_t, midi::Channel);
void sendNRPN(const struct Knob_t *, uint8_t, uint8_t, midi::Channel);
void sendProgramChange(uint8_t, midi::Channel);
void sendAfterTouch(const struct Knob_t *, uint8_t, uint8_t, midi::Channel);

void changeChannel(const bool &);
void changePreset(const bool &);

void buttonReleaseAction(const bool &);
void buttonPressAction(const bool &);
void renderButtonFunctions();

void doMidiRead();
uint8_t extractMode(const uint8_t &);
uint8_t extractChannel(const uint8_t &, const bool &);

#endif