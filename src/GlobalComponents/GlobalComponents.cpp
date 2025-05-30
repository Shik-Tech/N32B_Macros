/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#include <GlobalComponents/GlobalComponents.h>

void doMidiRead()
{
    MIDIUSB.read();
    MIDISerial1.read();
}
