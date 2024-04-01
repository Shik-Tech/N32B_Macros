/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "sysex.h"

void processSysex(unsigned char *data, unsigned int size)
{
    if (size > 3 && data[MANUFACTURER_INDEX] == SHIK_MANUFACTURER_ID)
    {
        switch (data[COMMAND_INDEX])
        {
        case SET_KNOB_MODE:
            device.activePreset.knobInfo[data[KNOB_INDEX]].MSB = data[MSB_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].LSB = data[LSB_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].MIN_A = data[MIN_A_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].MAX_A = data[MAX_A_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].MIN_B = data[MIN_B_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].MAX_B = data[MAX_B_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].CHANNELS = (data[CHANNEL_A_INDEX] << 4) | data[CHANNEL_B_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].OUTPUTS = (data[OUTPUT_A_INDEX] << 2) | data[OUTPUT_B_INDEX];
            device.activePreset.knobInfo[data[KNOB_INDEX]].PROPERTIES = (data[KNOB_MODE_INDEX] << 4) | data[PROPERTIES_INDEX];
            break;
        case SET_THRU_MODE:
            setMidiThruMode(data[KNOB_INDEX]);
            break;
        case SAVE_PRESET:
            savePreset(data[KNOB_INDEX]);
            break;
        case LOAD_PRESET:
            loadPreset(data[KNOB_INDEX]);
            break;
        case CHANGE_CHANNEL:
            handleChangeChannel(data[KNOB_INDEX]);
            break;
        case SEND_FIRMWARE_VERSION:
            sendDeviceFirmwareVersion();
            break;
        case SYNC_KNOBS:
            sendActivePreset();
            break;
        default:
            break;
        }
    }
}

void handleChangeChannel(byte channel)
{
    if (channel < 17 && channel > 0)
    {
        device.globalChannel = channel;
    }
}

// Change preset on program change
void handleProgramChange(byte channel, byte number)
{
    if (number < NUMBER_OF_PRESETS)
    {
        loadPreset(number);
    }
}

void sendDeviceFirmwareVersion()
{
    uint8_t data[5] = {SHIK_MANUFACTURER_ID, SEND_FIRMWARE_VERSION};

    // Send firmware version
    for (uint8_t i = 3; i > 0; i--)
    {
        data[i + 1] = EEPROM.read(EEPROM.length() - i);
    }
    MIDICoreUSB.sendSysEx(5, data);
}
void sendActivePreset()
{
    // Send current preset
    for (uint8_t i = 0; i < NUMBER_OF_KNOBS; i++)
    {
#ifdef N32Bv3
        uint8_t channel_a = device.activePreset.knobInfo[i].CHANNELS >> 4;
        uint8_t channel_b = device.activePreset.knobInfo[i].CHANNELS & 0xF;
        uint8_t output_a = device.activePreset.knobInfo[i].OUTPUTS >> 2;
        uint8_t output_b = device.activePreset.knobInfo[i].OUTPUTS & 0x3;
        uint8_t properties = device.activePreset.knobInfo[i].PROPERTIES & 0xF;
        uint8_t mode = device.activePreset.knobInfo[i].PROPERTIES >> 4;
        uint8_t knobsData[15] = {
            SHIK_MANUFACTURER_ID,
            SYNC_KNOBS,
            i,
            device.activePreset.knobInfo[i].MSB,
            device.activePreset.knobInfo[i].LSB,
            channel_a,
            channel_b,
            output_a,
            output_b,
            device.activePreset.knobInfo[i].MIN_A,
            device.activePreset.knobInfo[i].MAX_A,
            device.activePreset.knobInfo[i].MIN_B,
            device.activePreset.knobInfo[i].MAX_B,
            properties,
            mode};
        MIDICoreUSB.sendSysEx(15, knobsData);
#else
        uint8_t indexId = pgm_read_word_near(knobsLocation + i);
        uint8_t channel_a = device.activePreset.knobInfo[indexId].CHANNELS >> 4;
        uint8_t channel_b = device.activePreset.knobInfo[indexId].CHANNELS & 0xF;
        uint8_t output_a = device.activePreset.knobInfo[indexId].OUTPUTS >> 2;
        uint8_t output_b = device.activePreset.knobInfo[indexId].OUTPUTS & 0x3;
        uint8_t properties = device.activePreset.knobInfo[indexId].PROPERTIES & 0xF;
        uint8_t mode = device.activePreset.knobInfo[indexId].PROPERTIES & 0xF0;
        uint8_t knobsData[15] = {
            SHIK_MANUFACTURER_ID,
            SYNC_KNOBS,
            pgm_read_word_near(knobsLocation + i),
            device.activePreset.knobInfo[indexId].MSB,
            device.activePreset.knobInfo[indexId].LSB,
            channel_a,
            channel_b,
            output_a,
            output_b,
            device.activePreset.knobInfo[indexId].MIN_A,
            device.activePreset.knobInfo[indexId].MAX_A,
            device.activePreset.knobInfo[indexId].MIN_B,
            device.activePreset.knobInfo[indexId].MAX_B,
            properties,
            mode};
        MIDICoreUSB.sendSysEx(15, knobsData);
#endif
    }

    uint8_t presetThruData[3] = {
        SHIK_MANUFACTURER_ID,
        SET_THRU_MODE,
        device.activePreset.thruMode};

    MIDICoreUSB.sendSysEx(3, presetThruData);

    uint8_t endOfTransmissionData[2] = {
        SHIK_MANUFACTURER_ID,
        END_OF_TRANSMISSION};

    MIDICoreUSB.sendSysEx(2, endOfTransmissionData);
}
void setMidiThruMode(byte mode)
{
    device.activePreset.thruMode = mode;
}