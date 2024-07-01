/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#pragma once

#include <USB-MIDI.h>

USING_NAMESPACE_MIDI;
USING_NAMESPACE_USBMIDI;

struct CustomMidiSettings : public midi::DefaultSettings
{
    static const bool Use1ByteParsing = false;
};

class MidiUSBInterface
{
public:
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

class MidiSerialInterface1
{
public:
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