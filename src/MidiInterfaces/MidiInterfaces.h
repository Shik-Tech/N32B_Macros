/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#pragma once

#include <USB-MIDI.h>
#include "definitions.h"

USING_NAMESPACE_MIDI;
USING_NAMESPACE_USBMIDI;

/** Custom MIDI settings: multi-byte parsing, configurable SysEx max size. */
struct CustomMidiSettings : public midi::DefaultSettings
{
    static const bool Use1ByteParsing = false;
    static constexpr uint8_t SysExMaxSize = INTERFACE_SYSEX_MAX_LENGTH;
};

/** Singleton USB MIDI interface. */
class MidiUSBInterface
{
public:
    /** Returns the USB MIDI interface instance. */
    static MidiInterface<usbMidiTransport, CustomMidiSettings> &getInstance()
    {
        static usbMidiTransport usbMIDI(0);
        static MidiInterface<usbMidiTransport, CustomMidiSettings> instance(usbMIDI);
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copying
    MidiUSBInterface(MidiUSBInterface const &) = delete;
    void operator=(MidiUSBInterface const &) = delete;

private:
    MidiUSBInterface() {} // Private constructor
};

/** Singleton Serial1 MIDI interface. */
class MidiSerialInterface1
{
public:
    /** Returns the Serial1 MIDI interface instance. */
    static MidiInterface<SerialMIDI<HardwareSerial>, CustomMidiSettings> &getInstance()
    {
        static SerialMIDI<HardwareSerial> serialMIDI1(Serial1);
        static MidiInterface<SerialMIDI<HardwareSerial>, CustomMidiSettings> instance(serialMIDI1);
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copying
    MidiSerialInterface1(MidiSerialInterface1 const &) = delete;
    void operator=(MidiSerialInterface1 const &) = delete;

private:
    MidiSerialInterface1() {} // Private constructor
};