/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#pragma once

#include <USB-MIDI.h>
#include <MidiInterfaces/MidiInterfaces.h>
#include <display.h>
#include <adcMux.h>

// MIDI Interfaces
#define MIDIUSB MidiUSBInterface::getInstance()
#define MIDISerial1 MidiSerialInterface1::getInstance()

// Display
#define display Display::getInstance()

void doMidiRead();