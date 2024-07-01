#include "devices.h"

/* Device setup data */
Device_t device;
ADC_MUX muxFactory(device.pots);

#ifdef N32Bv3
N32B_DISPLAY n32b_display(SIN, SCLK, LAT, BLANK);
#else
N32B_DISPLAY n32b_display(DIN, CS, CLK);
#endif

USBMIDI_CREATE_INSTANCE(0, MIDICoreUSB);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDICoreSerial);