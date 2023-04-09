/*
  N32B Macros Firmware v4.0.0
  MIT License

  Copyright (c) 2023 SHIK
*/

#include "functions.h"

void onUsbMessage(const midi::Message<128> &message)
{
  if (device.activePreset.thruMode == THRU_TRS || device.activePreset.thruMode == THRU_BOTH)
  {
    MIDICoreSerial.send(message);
    n32b_display.blinkDot(2);
  }
}

void onSerialMessage(const midi::Message<128> &message)
{
  if (device.activePreset.thruMode == THRU_USB || device.activePreset.thruMode == THRU_BOTH)
  {
    if (MIDICoreSerial.getType() != midi::MidiType::ActiveSensing)
    {
      MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
      n32b_display.blinkDot(2);
    }
  }
}

void updateKnob(uint8_t index)
{
  Knob_t &currentKnob = device.activePreset.knobInfo[index];
  bool needToUpdate = false;
  uint16_t shiftedValue;
  uint8_t MSBValue;
  uint8_t LSBValue;
  if (extractMode(currentKnob.PROPERTIES) == KNOB_MODE_HIRES)
  {
    if (
        (device.knobValues[index][0] != device.knobValues[index][1]) &&
        (device.knobValues[index][0] != device.knobValues[index][2]) &&
        (device.knobValues[index][0] != device.knobValues[index][3]))
    {
      needToUpdate = true;
      shiftedValue = map(device.knobValues[index][0], 0, 1019, 0, 16383);
      MSBValue = shiftedValue >> 7;
      LSBValue = lowByte(shiftedValue) >> 1;
    }
  }
  else
  {
    shiftedValue = map(device.knobValues[index][0], 0, 1019, 0, 16383);
    MSBValue = shiftedValue >> 7;

    uint8_t BufferValue1 = map(device.knobValues[index][1], 0, 1019, 0, 16383) >> 7;
    uint8_t BufferValue2 = map(device.knobValues[index][2], 0, 1019, 0, 16383) >> 7;
    uint8_t BufferValue3 = map(device.knobValues[index][3], 0, 1019, 0, 16383) >> 7;
    if (
        (MSBValue != BufferValue1) &&
        (MSBValue != BufferValue2) &&
        (MSBValue != BufferValue3))
    {
      needToUpdate = true;
    }
  }

  if (needToUpdate)
  {
    uint8_t mode = extractMode(currentKnob.PROPERTIES);
    midi::Channel channel_a =
        bitRead(currentKnob.PROPERTIES, USE_OWN_CHANNEL_A_PROPERTY)
            ? extractChannel(currentKnob.CHANNELS, CHANNEL_A)
            : device.globalChannel;

    midi::Channel channel_b =
        bitRead(currentKnob.PROPERTIES, USE_OWN_CHANNEL_B_PROPERTY)
            ? extractChannel(currentKnob.CHANNELS, CHANNEL_B)
            : device.globalChannel;

    uint8_t normalizedMSBValue =
        map(MSBValue, 0, 127, currentKnob.MIN_A, currentKnob.MAX_A);

    uint8_t MSBSendValue =
        bitRead(currentKnob.PROPERTIES, INVERT_A_PROPERTY)
            ? currentKnob.MAX_A - normalizedMSBValue
            : normalizedMSBValue;

    uint8_t normalizedLSBValue =
        map(mode == KNOB_MODE_MACRO ? LSBValue : MSBValue, 0, 127, currentKnob.MIN_B, currentKnob.MAX_B);

    uint8_t LSBSendValue =
        bitRead(currentKnob.PROPERTIES, INVERT_B_PROPERTY)
            ? currentKnob.MAX_B - normalizedLSBValue
            : normalizedLSBValue;

    switch (mode)
    {
    case KNOB_MODE_STANDARD:
    case KNOB_MODE_HIRES:
      sendCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a);
      break;

    case KNOB_MODE_MACRO:
      sendMacroCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a, channel_b);
      break;

    case KNOB_MODE_NRPN:
      sendNRPM(currentKnob, MSBSendValue, channel_a);
      break;

    case KNOB_MODE_RPN:
      sendRPM(currentKnob, MSBSendValue, channel_a);
      break;

    default:
      break;
    }

    device.knobValues[index][3] = device.knobValues[index][2];
    device.knobValues[index][2] = device.knobValues[index][1];
    device.knobValues[index][1] = device.knobValues[index][0];
  }
}

void sendCCMessage(const struct Knob_t &currentKnob, uint8_t MSBvalue, uint8_t LSBvalue, midi::Channel channel)
{
  if (extractMode(currentKnob.PROPERTIES) == KNOB_MODE_HIRES)
  {
    MIDICoreSerial.sendControlChange(currentKnob.MSB, MSBvalue, channel);
    MIDICoreSerial.sendControlChange(currentKnob.LSB, LSBvalue, channel);

    MIDICoreUSB.sendControlChange(currentKnob.MSB, MSBvalue, channel);
    MIDICoreUSB.sendControlChange(currentKnob.LSB, LSBvalue, channel);
  }
  else
  {
    MIDICoreSerial.sendControlChange(currentKnob.MSB, MSBvalue, channel);
    MIDICoreUSB.sendControlChange(currentKnob.MSB, MSBvalue, channel);
  }
  n32b_display.blinkDot(1);
}

void sendMacroCCMessage(const struct Knob_t &currentKnob, uint8_t MSBvalue, uint8_t LSBvalue, midi::Channel channel_a, midi::Channel channel_b)
{
  MIDICoreSerial.sendControlChange(currentKnob.MSB, MSBvalue, channel_a);
  MIDICoreSerial.sendControlChange(currentKnob.LSB, LSBvalue, channel_b);

  MIDICoreUSB.sendControlChange(currentKnob.MSB, MSBvalue, channel_a);
  MIDICoreUSB.sendControlChange(currentKnob.LSB, LSBvalue, channel_b);

  n32b_display.blinkDot(1);
}

void sendNRPM(const struct Knob_t &currentKnob, uint8_t MSBvalue, midi::Channel channel)
{
  uint8_t MSBSendValue = bitRead(currentKnob.PROPERTIES, INVERT_A_PROPERTY) ? 127 - MSBvalue : MSBvalue;
  MIDICoreSerial.sendControlChange(99, currentKnob.MSB & 0x7F, channel); // NRPN MSB
  MIDICoreUSB.sendControlChange(99, currentKnob.MSB & 0x7F, channel);    // NRPN MSB

  MIDICoreSerial.sendControlChange(98, currentKnob.LSB & 0x7F, channel); // NRPN LSB
  MIDICoreUSB.sendControlChange(98, currentKnob.LSB & 0x7F, channel);    // NRPN LSB

  MIDICoreSerial.sendControlChange(6, MSBSendValue, channel); // Data Entry MSB
  MIDICoreUSB.sendControlChange(6, MSBSendValue, channel);    // Data Entry MSB

  n32b_display.blinkDot(1);
}

void sendRPM(const struct Knob_t &currentKnob, uint8_t MSBvalue, midi::Channel channel)
{
  uint8_t MSBSendValue = bitRead(currentKnob.PROPERTIES, INVERT_A_PROPERTY) ? 127 - MSBvalue : MSBvalue;
  MIDICoreSerial.sendControlChange(101, currentKnob.MSB & 0x7F, channel); // RPN MSB
  MIDICoreUSB.sendControlChange(101, currentKnob.MSB & 0x7F, channel);    // RPN MSB

  MIDICoreSerial.sendControlChange(100, currentKnob.LSB & 0x7F, channel); // RPN LSB
  MIDICoreUSB.sendControlChange(100, currentKnob.LSB & 0x7F, channel);    // RPN LSB

  MIDICoreSerial.sendControlChange(6, MSBSendValue, channel); // Data Entry MSB
  MIDICoreUSB.sendControlChange(6, MSBSendValue, channel);    // Data Entry MSB

  n32b_display.blinkDot(1);
}

void changeChannel(bool direction)
{
  if (direction)
  {
    // Next Channel
    if (device.globalChannel < 16)
      device.globalChannel++;
    else
      device.globalChannel = 1;
  }
  else
  {
    // Previous Channel
    if (device.globalChannel > 1)
      device.globalChannel--;
    else
      device.globalChannel = 16;
  }
}

void changePreset(bool direction)
{
  if (direction)
  {
    // Next Preset
    if (device.currentPresetIndex < NUMBER_OF_PRESETS - 1)
      loadPreset(device.currentPresetIndex + 1);
    else
      loadPreset(0);
  }
  else
  {
    // Previous Preset
    if (device.currentPresetIndex > 0)
      loadPreset(device.currentPresetIndex - 1);
    else
      loadPreset(NUMBER_OF_PRESETS - 1);
  }
  // MIDICoreSerial.sendProgramChange(device.currentPresetIndex, 1);
  // MIDICoreUSB.sendProgramChange(device.currentPresetIndex, 1);
}

void buttonReleaseAction(bool direction)
{
  direction ? isPressingAButton = false : isPressingBButton = false;

  if (millis() - pressedTime < SHORT_PRESS_TIME)
  {
    if (device.isPresetMode)
    {
      changePreset(direction);
      n32b_display.showPresetNumber(device.currentPresetIndex);
    }
    else
    {
      changeChannel(direction);
      n32b_display.showChannelNumber(device.globalChannel);
    }
  }
}

void buttonPressAction(bool direction)
{
  pressedTime = millis();
}

void renderButtonFunctions()
{
  // Must call the loop() function first
  buttonA.loop();
  buttonB.loop();

  if (buttonA.isPressed())
  {
    isPressingAButton = true;
    buttonPressAction(1);
  }

  if (buttonB.isPressed())
  {
    isPressingBButton = true;
    buttonPressAction(0);
  }

  if (buttonA.isReleased())
  {
    buttonReleaseAction(1);
  }

  if (buttonB.isReleased())
  {
    buttonReleaseAction(0);
  }

  // Switch between channelMode and presetMode
  if (
      (isPressingAButton || isPressingBButton) &&
      (millis() - pressedTime > (unsigned int)(SHORT_PRESS_TIME << 2)))
  {
    if (isPressingAButton)
    {
      device.isPresetMode = false;
      n32b_display.showChannelNumber(device.globalChannel);
    }
    if (isPressingBButton)
    {
      device.isPresetMode = true;
      n32b_display.showPresetNumber(device.currentPresetIndex);
    }
  }
}

void doMidiRead()
{
  MIDICoreSerial.read();
  MIDICoreUSB.read();
}

uint8_t extractMode(uint8_t properties)
{
  return (properties & B01110000) >> MODE_PROPERTY;
}
uint8_t extractChannel(uint8_t channels, bool isA)
{
  uint8_t outChannel = channels & 0xF;
  if (isA)
  {
    outChannel = (channels & 0xF0) >> 4;
  }
  return outChannel + 1;
}