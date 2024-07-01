/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#pragma once

#include <Arduino.h>
// #include "definitions.h"
#include "storage.h"
#include <MidiInterfaces/MidiInterfaces.h>

USING_NAMESPACE_MIDI;

void onUsbMessage(const midi::Message<128> &);
void onSerialMessage(const midi::Message<128> &);

void updateKnob(uint8_t &index, bool force = false);

void invertValue(uint8_t, uint8_t, uint8_t &, uint8_t &, midi::DataByte *);
void scaleValuesByRange(uint16_t, uint8_t &, uint8_t &, midi::DataByte *, bool);
void sendMidiMessage(uint8_t &index, bool force = false);

template <typename Transport>
void sendNrpnMidiMessage(midi::MidiInterface<Transport, CustomMidiSettings> &MidiInterface, uint8_t &msbNumber, uint8_t &lsbNumber, midi::DataByte &MSB, midi::DataByte &LSB, midi::Channel &channel);

template <typename Transport>
void sendRpnMidiMessage(midi::MidiInterface<Transport, CustomMidiSettings> &MidiInterface, uint8_t &msbNumber, uint8_t &lsbNumber, midi::DataByte &MSB, midi::DataByte &LSB, midi::Channel &channel);

void sendStandardCCMessage(uint8_t, uint8_t, uint8_t, midi::Channel);

void changeChannel(bool);
void changePreset(bool);

void extractMode(uint8_t properties, uint8_t *mode);
void extractChannels(uint8_t data, uint8_t properties, midi::Channel *channel_a, midi::Channel *channel_b);
void extractOutputs(uint8_t outputs, uint8_t *output_a, uint8_t *output_b);

void handleButtons();
void changeChannel(bool direction);
void changePreset(bool direction);
void sendSnapshot();