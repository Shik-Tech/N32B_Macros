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

void updateKnob(uint8_t);

void sendCCMessage(const struct Knob_t &, uint8_t, uint8_t, midi::Channel);
void sendMacroCCMessage(const struct Knob_t &, uint8_t, uint8_t, midi::Channel, midi::Channel);
void sendRPM(const struct Knob_t &, uint8_t, midi::Channel);
void sendNRPM(const struct Knob_t &, uint8_t, midi::Channel);

void changeChannel(bool);
void changePreset(bool);

void buttonReleaseAction(bool);
void buttonPressAction(bool);
void renderButtonFunctions();

void doMidiRead();
uint8_t extractMode(uint8_t);
uint8_t extractChannel(uint8_t, bool);

#endif