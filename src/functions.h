/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
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

void invertValue(uint8_t, uint8_t, uint8_t *);
void scaleValuesByRange(uint16_t, uint8_t &, uint8_t &, uint8_t *, bool);
void sendMidiMessage(const uint8_t &index);
void sendStandardCCMessage(uint8_t, uint8_t, uint8_t, midi::Channel);

void changeChannel(bool);
void changePreset(bool);

void buttonReleaseAction(const bool &);
void buttonPressAction(const bool &);
void renderButtonFunctions();

void doMidiRead();
uint8_t extractMode(const uint8_t &);
uint8_t extractChannel(const uint8_t &, const bool &);
uint8_t extractOutputs(const uint8_t &, const bool &);

#endif