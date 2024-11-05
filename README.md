# S32-Firmware

This repository contains the firmware for the N32B MIDI controller, designed around a Microchip SAMD21 microcontroller.

## Dependencies

### Libraries

| Library | Description | URL | URL (Base Project) |
| :--- | :--- | :--- | :--- |
| ArduinoCore-samd | Arduino Core for SAMD21 CPU | https://github.com/Shik-Tech/ArduinoCore-samd.git | https://github.com/arduino/ArduinoCore-samd.git |
| arduino_midi_library | Arduino MIDI Library | https://github.com/FortySevenEffects/arduino_midi_library.git | |
| Arduino-USBMIDI | Arduino USB-MIDI Transport | https://github.com/lathoub/Arduino-USBMIDI.git | |
| JC_EEPROM | Arduino EEPROM Library | https://github.com/JChristensen/JC_EEPROM.git | |
| MIDIUSB | MIDIUSB Library for Arduino | https://github.com/arduino-libraries/MIDIUSB.git | |

## Build Tools

### PlatformIO

https://platformio.org/

### UF2CONV

The UF2CONV tool is required to generate a `.uf2` file from the built firmware for updating the device via a UF2 bootloader.  
It's available both as a C program and as a Python script. We currently use the Python script, `uf2conv.py`,
which can be found in the `utils/` subdirectory of the following repository:

| Repository | Description | URL |
| :--- | :--- | :--- |
| uf2 | USB Flashing Format (UF2) | https://github.com/microsoft/uf2.git |

A `.uf2` file can be generated from a `.bin` object file using the following command:

```
python uf2conv.py -f SAMD21 -b 0x2000 --convert <file.bin> -o <file.uf2>
```

As explained below, it is automatically invoked when building the firmware using the PlatformIO project, so the above command
is given for reference.

## Setting Up the PlatformIO Project

The following files located at the root of the firmware project include the additional steps to generate the `.uf2` file automatically
when building the firmware:

* `extra_script.py`
* `platformio.ini`

The `platformio.ini` file needs to be modified according to the local installation on the build machine, as it invokes
the `uf2conv.py` mentioned above. The following section needs to be adapted:

```
[custom_tools]
uf2conv = python /path/to/uf2conv.py
uf2_family = SAMD21
; Base address in flash memory.
uf2_baseaddr = 0x2000
```
Replace the `/path/to/uf2conv.py` part with the actual file path where the `uf2conv.py` is located.

**Note**: if the `uf2conv.py` is copied manually to some local directory, the `uf2families.json` (that is located in the same directory
in the repository mentioned in the [UF2CONV](#UF2CONV) paragraph above) needs to be copied alongside it.

## UF2 Bootloader

A UF2 bootloader is used to allow easy updating of the firmware in the field. It can be found in the following repository:

| Repository | Description | URL | URL (Base Project) |
| :--- | :--- | :--- | :--- |
| uf2-samdx1 | UF2 Bootloader | https://github.com/Shik-Tech/uf2-samdx1.git | https://github.com/adafruit/uf2-samdx1.git |

It has been adapted to the hardware of the S32 series. The current variant is `shik-n32b-samd21` and can be found in
the `boards/` subdirectory.

### Building the Bootloader

To build the bootloader, navigate to the UF2 bootloader root directory, and issue the following command:

```
make BOARD=shik-n32b-samd21
```

The binaries will be generated in the `build/shik-n32b-samd21/` subdirectory with the following file names:

`bootloader-shik-n32b-samd21-v*.*`

