/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#pragma once

#include <Arduino.h>
#include "definitions.h"
#include "devices.h"
#include "storage.h"
#include <MidiInterfaces/MidiInterfaces.h>

USING_NAMESPACE_MIDI;

/** MIDI thru callback for USB input. Routes according to active thruMode. */
void onUsbMessage(const midi::Message<128> &);

/** MIDI thru callback for TRS serial input. Routes according to active thruMode. */
void onSerialMessage(const midi::Message<128> &);

/**
 * Polls pot state; sends MIDI if in motion, resets display if idle.
 *
 * @param index Knob index (0–31).
 * @param force If true, send regardless of change detection.
 */
void updateKnob(uint8_t index, bool force = false);

/** Inverts value within [min, max] when the property bit at invertIndex is set. */
void invertValue(uint8_t, uint8_t, uint8_t &, uint8_t &, midi::DataByte *);

/**
 * Scales a 14-bit ADC value to a knob's min–max range.
 *
 * @param isLSB If true, outputs LSB (lower 7 bits); otherwise MSB (upper 7 bits).
 */
void scaleValuesByRange(uint16_t, uint8_t &, uint8_t &, midi::DataByte *, bool isLSB = false);

/**
 * Sends MIDI for a knob based on its configured mode (CC, HiRes, Macro, NRPN, RPN,
 * Program Change, Aftertouch).
 *
 * @param index Knob index (0–31).
 * @param force If true, send regardless of change detection.
 */
void sendMidiMessage(uint8_t index, bool force = false);

/** Sends an NRPN message (begin/value/end) via the given MIDI transport. */
template <typename Transport>
void sendNrpnMidiMessage(midi::MidiInterface<Transport, CustomMidiSettings> &MidiInterface, uint8_t &msbNumber, uint8_t &lsbNumber, midi::DataByte &MSB, midi::DataByte &LSB, midi::Channel &channel);

/** Sends an RPN message (begin/value/end) via the given MIDI transport. */
template <typename Transport>
void sendRpnMidiMessage(midi::MidiInterface<Transport, CustomMidiSettings> &MidiInterface, uint8_t &msbNumber, uint8_t &lsbNumber, midi::DataByte &MSB, midi::DataByte &LSB, midi::Channel &channel);

/** Sends a Control Change message to TRS and/or USB per the output mode. */
void sendStandardCCMessage(uint8_t, uint8_t, uint8_t, midi::Channel);

/** Extracts the knob mode from properties bits 4–7. */
void extractMode(uint8_t properties, uint8_t *mode);

/** Extracts channel A/B from packed data byte, using global channel as fallback. */
void extractChannels(uint8_t data, uint8_t properties, midi::Channel *channel_a, midi::Channel *channel_b);

/** Extracts output A/B routing from the packed outputs byte. */
void extractOutputs(uint8_t outputs, uint8_t *output_a, uint8_t *output_b);

/**
 * Debounces both buttons and dispatches short-press (channel/preset change)
 * and long-press (mode switch / snapshot) actions.
 *
 * @note Not ISR-safe — call from main loop only.
 */
void handleButtons();

/** Increments or decrements the global MIDI channel (1–16, wrapping). */
void changeChannel(bool direction);

/** Loads the next or previous preset (wrapping at boundaries). */
void changePreset(bool direction);

/**
 * Forces an ADC read of all 32 knobs, then sends the current value of each.
 *
 * @note Blocking — iterates all knobs twice (ADC read pass + MIDI send pass).
 */
void sendSnapshot();