/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#include "sysex.h"
#include "functions.h"
#include <GlobalComponents/GlobalComponents.h>

void processSysex(unsigned char *data, unsigned int size)
{
    if (size < kSysexMinSize_NoPayload || data[MANUFACTURER_INDEX] != SHIK_MANUFACTURER_ID)
        return;

    switch (data[COMMAND_INDEX])
    {
    case SET_KNOB_MODE:
        if (size >= kSysexMinSize_SET_KNOB_MODE && data[KNOB_INDEX] < NUMBER_OF_KNOBS)
        {
            const uint8_t k = data[KNOB_INDEX];
            device.activePreset.knobInfo[k].MSB = data[MSB_INDEX];
            device.activePreset.knobInfo[k].LSB = data[LSB_INDEX];
            device.activePreset.knobInfo[k].MIN_A = data[MIN_A_INDEX];
            device.activePreset.knobInfo[k].MAX_A = data[MAX_A_INDEX];
            device.activePreset.knobInfo[k].MIN_B = data[MIN_B_INDEX];
            device.activePreset.knobInfo[k].MAX_B = data[MAX_B_INDEX];
            device.activePreset.knobInfo[k].CHANNELS = (data[CHANNEL_A_INDEX] << 4) | data[CHANNEL_B_INDEX];
            device.activePreset.knobInfo[k].OUTPUTS = (data[OUTPUT_A_INDEX] << 2) | data[OUTPUT_B_INDEX];
            device.activePreset.knobInfo[k].PROPERTIES = (data[KNOB_MODE_INDEX] << 4) | data[PROPERTIES_INDEX];
        }
        break;
    case SET_THRU_MODE:
        if (size >= kSysexMinSize_PresetOrThruOrChannel)
            setMidiThruMode(data[KNOB_INDEX]);
        break;
    case SAVE_PRESET:
        if (size >= kSysexMinSize_PresetOrThruOrChannel && data[KNOB_INDEX] < NUMBER_OF_PRESETS)
            savePreset(data[KNOB_INDEX]);
        break;
    case LOAD_PRESET:
        if (size >= kSysexMinSize_PresetOrThruOrChannel && data[KNOB_INDEX] < NUMBER_OF_PRESETS)
            loadPreset(data[KNOB_INDEX]);
        break;
    case CHANGE_CHANNEL:
        if (size >= kSysexMinSize_PresetOrThruOrChannel)
            handleChangeChannel(data[KNOB_INDEX]);
        break;
    case SEND_FIRMWARE_VERSION:
        sendDeviceFirmwareVersion();
        break;
    case SYNC_KNOBS:
        sendActivePreset();
        break;
    case SEND_SNAPSHOT:
        sendSnapshot();
        break;
    default:
        break;
    }
}

void handleChangeChannel(byte channel)
{
    if (channel < 17 && channel > 0)
    {
        device.globalChannel = channel;
    }
}

void handleProgramChange(byte channel, byte number)
{
    (void)channel;
    if (number < NUMBER_OF_PRESETS)
        loadPreset(number);
}

void sendDeviceFirmwareVersion()
{
    uint8_t data[5] = {SHIK_MANUFACTURER_ID, SEND_FIRMWARE_VERSION};

    // Send firmware version
    for (uint8_t i = 3; i > 0; i--)
    {
        data[i + 1] = EEPROM.read(EEPROM.length() - i);
    }
    MIDIUSB.sendSysEx(5, data);
    MIDISerial1.sendSysEx(5, data);
}
void sendActivePreset()
{
    for (uint8_t i = 0; i < NUMBER_OF_KNOBS; i++)
    {
#ifdef N32Bv3
        const uint8_t indexId = i;
#else
        const uint8_t indexId = pgm_read_byte_near(knobsLocation + i);
#endif
        const Knob_t &knob = device.activePreset.knobInfo[indexId];
        uint8_t channel_a = knob.CHANNELS >> 4;
        uint8_t channel_b = knob.CHANNELS & 0xF;
        uint8_t output_a = knob.OUTPUTS >> 2;
        uint8_t output_b = knob.OUTPUTS & 0x3;
        uint8_t properties = knob.PROPERTIES & 0xF;
        uint8_t mode = knob.PROPERTIES >> 4;
        uint8_t knobsData[15] = {
            SHIK_MANUFACTURER_ID,
            SYNC_KNOBS,
            indexId,
            knob.MSB,
            knob.LSB,
            channel_a,
            channel_b,
            output_a,
            output_b,
            knob.MIN_A,
            knob.MAX_A,
            knob.MIN_B,
            knob.MAX_B,
            properties,
            mode};
        MIDIUSB.sendSysEx(15, knobsData);
        MIDISerial1.sendSysEx(15, knobsData);
    }

    uint8_t presetThruData[3] = {
        SHIK_MANUFACTURER_ID,
        SET_THRU_MODE,
        device.activePreset.thruMode};

    MIDIUSB.sendSysEx(3, presetThruData);
    MIDISerial1.sendSysEx(3, presetThruData);

    uint8_t endOfTransmissionData[2] = {
        SHIK_MANUFACTURER_ID,
        END_OF_TRANSMISSION};

    MIDIUSB.sendSysEx(2, endOfTransmissionData);
    MIDISerial1.sendSysEx(2, endOfTransmissionData);
}
void setMidiThruMode(byte mode)
{
    if (mode <= THRU_BOTH_DIRECTIONS)
        device.activePreset.thruMode = mode;
}