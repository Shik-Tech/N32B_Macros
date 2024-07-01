/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#pragma once

#include <Arduino.h>
#include <USB-MIDI.h>
#include "definitions.h"
#include "adcMux.h"
#include "display.h"
#include "sysex.h"
#include "pot.h"

USING_NAMESPACE_MIDI;

extern Device_t device;
extern ADC_MUX muxFactory;
extern N32B_DISPLAY n32b_display;
extern MidiInterface<USBMIDI_NAMESPACE::usbMidiTransport> MIDICoreUSB;
extern MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDICoreSerial;