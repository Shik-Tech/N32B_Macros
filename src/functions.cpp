/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "functions.h"

void onUsbMessage(const midi::Message<128> &message)
{
  if (message.type != ActiveSensing)
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
      break;
    }
    n32b_display.blinkDot(2);
  }
}

void onSerialMessage(const midi::Message<128> &message)
{
  if (message.type != ActiveSensing)
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
      break;
    }
    n32b_display.blinkDot(2);
  }
}

void updateKnob(const uint8_t &index)
{
  Pot &pot = device.pots[index];
  if (pot.getState() == Pot_t::IN_MOTION)
  {
    sendMidiMessage(index);
  }
}
void invertValue(uint8_t properties, uint8_t invertIndex, uint8_t &max, uint8_t &min, uint8_t *value)
{
  if (bitRead(properties, invertIndex))
  {
    *value = max - (*value - min);
  }
}
void scaleValuesByRange(uint16_t value, uint8_t &max, uint8_t &min, uint8_t *outputValue, bool isLSB = false)
{
  // Define the total range based on the min and max settings
  uint16_t totalRange = ((max - min + 1) << 7);

  // Scale the 14-bit value to the defined total range
  uint32_t scaledValue = ((uint32_t)value * totalRange) >> 14;
  if (scaledValue > 16383)
    scaledValue = 16383; // Cap the total range to 14-bit max if it exceeds

  // Calculate the MSB or LSB from the scaled value
  *outputValue = isLSB ? scaledValue & 0x7F : (scaledValue >> 7) & 0x7F; // Get the top 7 bits as MSB

  // Adjust output based on the min to ensure it starts from the defined minimum
  *outputValue += min;
}

void sendMidiMessage(const uint8_t &index)
{
  // if (isStartup)
  //       return;

  Knob_t &knob = device.activePreset.knobInfo[index];
  Pot &pot = device.pots[index];

  const uint16_t value_14bit = pot.getCurrentValue();
  const uint16_t prev_value_14bit = pot.getPreviousValue();
  uint8_t oldMSB = (prev_value_14bit >> 7) & 0x7F;
  uint8_t oldLSB = prev_value_14bit & 0x7F;
  // uint8_t MSB = (value_14bit >> 7) & 0x7F;
  // uint8_t LSB = value_14bit & 0x7F;
  uint8_t MSB;
  uint8_t LSB;
  uint8_t mode = extractMode(knob.PROPERTIES);
  midi::Channel channel_a =
      bitRead(knob.PROPERTIES, USE_OWN_CHANNEL_A_PROPERTY)
          ? extractChannel(knob.CHANNELS, CHANNEL_A)
          : device.globalChannel;
  midi::Channel channel_b =
      bitRead(knob.PROPERTIES, USE_OWN_CHANNEL_B_PROPERTY)
          ? extractChannel(knob.CHANNELS, CHANNEL_B)
          : device.globalChannel;
  uint8_t macro_a_output = extractOutputs(knob.OUTPUTS, true);
  uint8_t macro_b_output = extractOutputs(knob.OUTPUTS, false);

  bool isMidiChanged = false;

  scaleValuesByRange(value_14bit, knob.MAX_A, knob.MIN_A, &MSB);

  if (extractMode(knob.PROPERTIES) == KNOB_MODE_HIRES)
  {
    scaleValuesByRange(value_14bit, knob.MAX_B, knob.MIN_B, &LSB, true);
    invertValue(knob.PROPERTIES, INVERT_A_PROPERTY, knob.MAX_A, knob.MIN_A, &MSB);

    if (bitRead(knob.PROPERTIES, INVERT_A_PROPERTY))
    {
      LSB = 127 - LSB;
    }
  }
  else
  {
    scaleValuesByRange(value_14bit, knob.MAX_B, knob.MIN_B, &LSB);
    invertValue(knob.PROPERTIES, INVERT_A_PROPERTY, knob.MAX_A, knob.MIN_A, &MSB);
    invertValue(knob.PROPERTIES, INVERT_B_PROPERTY, knob.MAX_B, knob.MIN_B, &LSB);
  }

  switch (mode)
  {
  case KNOB_MODE_STANDARD:
    if (oldMSB != MSB)
    {
      sendStandardCCMessage(macro_a_output, knob.MSB, MSB, channel_a);
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_HIRES:
    if (oldLSB != LSB)
    {
      sendStandardCCMessage(macro_a_output, knob.MSB, MSB, channel_a);
      sendStandardCCMessage(macro_a_output, knob.LSB, LSB, channel_a);

      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_MACRO:
    if (oldMSB != MSB)
    {
      sendStandardCCMessage(macro_a_output, knob.MSB, MSB, channel_a);
      sendStandardCCMessage(macro_b_output, knob.LSB, LSB, channel_b);

      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_NRPN:
    if (oldLSB != LSB)
    {
      if (macro_a_output == OUTPUT_TRS ||
          macro_a_output == OUTPUT_BOTH)
      {
        MIDICoreSerial.beginNrpn(knob.MSB << 7 | knob.LSB, channel_a);
        MIDICoreSerial.sendNrpnValue(MSB, LSB, channel_a);
        MIDICoreSerial.endNrpn(channel_a);
      }
      if (macro_a_output == OUTPUT_USB ||
          macro_a_output == OUTPUT_BOTH)
      {

        MIDICoreUSB.beginNrpn(knob.MSB << 7 | knob.LSB, channel_a);
        MIDICoreUSB.sendNrpnValue(MSB, LSB, channel_a);
        MIDICoreUSB.endNrpn(channel_a);
      }
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_RPN:
    if (oldLSB != LSB)
    {
      if (macro_a_output == OUTPUT_TRS || macro_a_output == OUTPUT_BOTH)
      {
        MIDICoreSerial.beginRpn(knob.MSB << 7 | knob.LSB, channel_a);
        MIDICoreSerial.sendRpnValue(MSB, LSB, channel_a);
        MIDICoreSerial.endRpn(channel_a);
      }
      if (macro_a_output == OUTPUT_USB || macro_a_output == OUTPUT_BOTH)
      {
        MIDICoreUSB.beginRpn(knob.MSB << 7 | knob.LSB, channel_a);
        MIDICoreUSB.sendRpnValue(MSB, LSB, channel_a);
        MIDICoreUSB.endRpn(channel_a);
      }
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_PROGRAM_CHANGE:
    if (oldMSB != MSB)
    {
      if (macro_a_output == OUTPUT_TRS || macro_a_output == OUTPUT_BOTH)
      {
        MIDICoreSerial.sendProgramChange(MSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB || macro_a_output == OUTPUT_BOTH)
      {
        MIDICoreUSB.sendProgramChange(MSB, channel_a);
      }

      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_POLY_AFTER_TOUCH:
    if (oldMSB != MSB)
    {
      if (macro_a_output == OUTPUT_TRS || macro_a_output == OUTPUT_BOTH)
      {
        MIDICoreSerial.sendAfterTouch(knob.MSB, MSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB || macro_a_output == OUTPUT_BOTH)
      {
        MIDICoreUSB.sendAfterTouch(knob.MSB, MSB, channel_a);
      }
      isMidiChanged = true;
    }
    break;
  case KNOB_MODE_MONO_AFTER_TOUCH:
    if (oldMSB != MSB)
    {
      if (macro_a_output == OUTPUT_TRS || macro_a_output == OUTPUT_BOTH)
      {
        MIDICoreSerial.sendAfterTouch(MSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB || macro_a_output == OUTPUT_BOTH)
      {
        MIDICoreUSB.sendAfterTouch(MSB, channel_a);
      }
      isMidiChanged = true;
    }
    break;
  }

  if (isMidiChanged)
  {
#ifndef N32Bv3
    n32b_display.blinkDot(1);
#else
    n32b_display.showValue(MSB);
#endif
  }

  pot.setPreviousValue();
}

void sendStandardCCMessage(uint8_t output, uint8_t message, uint8_t value, midi::Channel channel)
{
  if (output == OUTPUT_TRS || output == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendControlChange(message, value, channel);
  }
  if (output == OUTPUT_USB || output == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendControlChange(message, value, channel);
  }
}

void changeChannel(bool direction)
{
  if (direction)
  {
    // Next Channel
    device.globalChannel = (device.globalChannel % 16) + 1;
  }
  else
  {
    // Previous Channel
    device.globalChannel = (device.globalChannel - 2 + 16) % 16 + 1;
  }
  n32b_display.showChannelNumber(device.globalChannel);
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
}

void buttonReleaseAction(const bool &direction)
{
  direction ? isPressingAButton = false : isPressingBButton = false;

  if (millis() - pressedTime < SHORT_PRESS_TIME)
    device.isPresetMode ? changePreset(direction) : changeChannel(direction);
}

void buttonPressAction(const bool &direction)
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

uint8_t extractMode(const uint8_t &properties)
{
  return (properties & B01110000) >> 4;
}
uint8_t extractChannel(const uint8_t &data, const bool &isMacroA)
{
  return (isMacroA ? (data & 0xF0) >> 4 : data & 0xF) + 1;
}
uint8_t extractOutputs(const uint8_t &outputs, const bool &isMacroA)
{
  return isMacroA ? (outputs & 0xC) >> 2 : outputs & 0x3;
}