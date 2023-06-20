/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2023 SHIK
*/

#include "functions.h"

void onUsbMessage(const midi::Message<128> &message)
{
  if (message.type != midi::MidiType::ActiveSensing)
  {
    switch (device.activePreset.thruMode)
    {
    case THRU_USB_USB:
      MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
      break;

    case THRU_USB_TRS:
      MIDICoreSerial.send(message);
      break;
    case THRU_BOTH_DIRECTIONS:
      MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
      MIDICoreSerial.send(message);
    }
    n32b_display.blinkDot(2);
  }
}

void onSerialMessage(const midi::Message<128> &message)
{
  if (message.type != midi::MidiType::ActiveSensing)
  {
    switch (device.activePreset.thruMode)
    {
    case THRU_TRS_TRS:
      MIDICoreSerial.send(message);
      break;

    case THRU_TRS_USB:
      MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
      break;
    case THRU_BOTH_DIRECTIONS:
      MIDICoreUSB.send(message.type, message.data1, message.data2, message.channel);
      MIDICoreSerial.send(message);
    }
    n32b_display.blinkDot(2);
  }
}

void updateKnob(uint8_t index)
{
  Knob_t &currentKnob = device.activePreset.knobInfo[index];
  bool needToUpdate = false;
  uint16_t shiftedValue;
  uint16_t oldShiftedValue;
  uint8_t MSBValue;
  uint8_t oldMSBValue;
  uint8_t LSBValue;
  uint8_t oldLSBValue;

  shiftedValue = map(device.knobValues[index][0], 0, 1023, 0, 16383);
  oldShiftedValue = map(device.knobValues[index][2], 0, 1023, 0, 16383);
  MSBValue = shiftedValue >> 7;
  oldMSBValue = oldShiftedValue >> 7;
  LSBValue = lowByte(shiftedValue) >> 1;
  oldLSBValue = lowByte(oldShiftedValue) >> 1;

  if (extractMode(currentKnob.PROPERTIES) == KNOB_MODE_HIRES)
  {
    if ((device.knobValues[index][0] != device.knobValues[index][2]) && LSBValue != oldLSBValue)
    {
      needToUpdate = true;
    }
  }
  else
  {
    if ((device.knobValues[index][0] != device.knobValues[index][2]) && MSBValue != oldMSBValue)
    {
      needToUpdate = true;
    }
  }

  if (needToUpdate)
  {
    device.knobValues[index][2] = device.knobValues[index][0];

    uint8_t mode = extractMode(currentKnob.PROPERTIES);
    midi::Channel channel_a =
        bitRead(currentKnob.PROPERTIES, USE_OWN_CHANNEL_A_PROPERTY)
            ? extractChannel(currentKnob.CHANNELS, CHANNEL_A)
            : device.globalChannel;

    midi::Channel channel_b =
        bitRead(currentKnob.PROPERTIES, USE_OWN_CHANNEL_B_PROPERTY)
            ? extractChannel(currentKnob.CHANNELS, CHANNEL_B)
            : device.globalChannel;

    uint8_t MSBSendValue = map(MSBValue, 0, 127, currentKnob.MIN_A, currentKnob.MAX_A);
    if (bitRead(currentKnob.PROPERTIES, INVERT_A_PROPERTY))
    {
      MSBSendValue = map(MSBValue, 0, 127, currentKnob.MAX_A, currentKnob.MIN_A);
    }

    uint8_t LSBSendValue;
    if (extractMode(currentKnob.PROPERTIES) == KNOB_MODE_HIRES)
    {
      LSBSendValue = LSBValue;
      if (bitRead(currentKnob.PROPERTIES, INVERT_A_PROPERTY))
      {
        LSBSendValue = 127 - LSBValue;
      }
    }
    else
    {
      LSBSendValue = map(MSBValue, 0, 127, currentKnob.MIN_B, currentKnob.MAX_B);
      if (bitRead(currentKnob.PROPERTIES, INVERT_B_PROPERTY))
      {
        LSBSendValue = map(MSBValue, 0, 127, currentKnob.MAX_B, currentKnob.MIN_B);
      }
    }

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
  }
}

void sendCCMessage(const struct Knob_t &currentKnob, uint8_t MSBvalue, uint8_t LSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendControlChange(currentKnob.MSB, MSBvalue, channel);
    if (extractMode(currentKnob.PROPERTIES) == KNOB_MODE_HIRES)
    {
      MIDICoreSerial.sendControlChange(currentKnob.LSB, LSBvalue, channel);
    }
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendControlChange(currentKnob.MSB, MSBvalue, channel);
    if (extractMode(currentKnob.PROPERTIES) == KNOB_MODE_HIRES)
    {
      MIDICoreUSB.sendControlChange(currentKnob.LSB, LSBvalue, channel);
    }
  }
  n32b_display.blinkDot(1);
}

void sendMacroCCMessage(const struct Knob_t &currentKnob, uint8_t MSBvalue, uint8_t LSBvalue, midi::Channel channel_a, midi::Channel channel_b)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendControlChange(currentKnob.MSB, MSBvalue, channel_a);
    MIDICoreSerial.sendControlChange(currentKnob.LSB, LSBvalue, channel_b);
  }

  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendControlChange(currentKnob.MSB, MSBvalue, channel_a);
    MIDICoreUSB.sendControlChange(currentKnob.LSB, LSBvalue, channel_b);
  }
  n32b_display.blinkDot(1);
}

void sendNRPM(const struct Knob_t &currentKnob, uint8_t MSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendControlChange(99, currentKnob.MSB & 0x7F, channel); // NRPN MSB
    MIDICoreSerial.sendControlChange(98, currentKnob.LSB & 0x7F, channel); // NRPN LSB
    MIDICoreSerial.sendControlChange(6, MSBvalue, channel);                // Data Entry MSB
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendControlChange(99, currentKnob.MSB & 0x7F, channel); // NRPN MSB
    MIDICoreUSB.sendControlChange(98, currentKnob.LSB & 0x7F, channel); // NRPN LSB
    MIDICoreUSB.sendControlChange(6, MSBvalue, channel);                // Data Entry MSB
  }
  n32b_display.blinkDot(1);
}

void sendRPM(const struct Knob_t &currentKnob, uint8_t MSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendControlChange(101, currentKnob.MSB & 0x7F, channel); // RPN MSB
    MIDICoreSerial.sendControlChange(100, currentKnob.LSB & 0x7F, channel); // RPN LSB
    MIDICoreSerial.sendControlChange(6, MSBvalue, channel);                 // Data Entry MSB
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendControlChange(101, currentKnob.MSB & 0x7F, channel); // RPN MSB
    MIDICoreUSB.sendControlChange(100, currentKnob.LSB & 0x7F, channel); // RPN LSB
    MIDICoreUSB.sendControlChange(6, MSBvalue, channel);                 // Data Entry MSB
  }
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