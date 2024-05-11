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

void updateKnob(uint8_t &index)
{
  Pot &pot = device.pots[index];
  if (pot.getState() == Pot_t::IN_MOTION)
  {
    sendMidiMessage(index);
  }
}
void invertValue(uint8_t properties, uint8_t invertIndex, uint8_t &max, uint8_t &min, midi::DataByte *value)
{
  if (bitRead(properties, invertIndex))
  {
    *value = max - (*value - min);
  }
}
void scaleValuesByRange(uint16_t value, uint8_t &max, uint8_t &min, midi::DataByte *outputValue, bool isLSB = false)
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

void sendMidiMessage(uint8_t &index)
{
  // if (isStartup)
  //       return;

  Knob_t &knob = device.activePreset.knobInfo[index];
  Pot &pot = device.pots[index];

  uint16_t value_14bit = pot.getCurrentValue();
  uint16_t prev_value_14bit = pot.getPreviousValue();
  uint8_t oldMSB;
  uint8_t oldLSB;
  uint8_t MSB;
  uint8_t LSB;
  uint8_t mode;
  midi::Channel channel_a;
  midi::Channel channel_b;
  uint8_t macro_a_output;
  uint8_t macro_b_output;

  bool isMidiChanged = false;

  extractMode(knob.PROPERTIES, &mode);
  extractChannels(knob.CHANNELS, knob.PROPERTIES, &channel_a, &channel_b);
  extractOutputs(knob.OUTPUTS, &macro_a_output, &macro_b_output);
  scaleValuesByRange(value_14bit, knob.MAX_A, knob.MIN_A, &MSB);
  scaleValuesByRange(prev_value_14bit, knob.MAX_A, knob.MIN_A, &oldMSB);

  if (mode == KNOB_MODE_HIRES)
  {
    scaleValuesByRange(value_14bit, knob.MAX_B, knob.MIN_B, &LSB, true); // TODO: check if this should be MAX_A, MIN_B instead!
    scaleValuesByRange(prev_value_14bit, knob.MAX_B, knob.MIN_B, &oldLSB, true); // TODO: check if this should be MAX_A, MIN_B instead!
    invertValue(knob.PROPERTIES, INVERT_A_PROPERTY, knob.MAX_A, knob.MIN_A, &MSB);
    invertValue(knob.PROPERTIES, INVERT_A_PROPERTY, knob.MAX_A, knob.MIN_A, &oldMSB);

    if (bitRead(knob.PROPERTIES, INVERT_A_PROPERTY))
    {
      LSB = 127 - LSB;
      oldLSB = 127 - oldLSB;
    }
  }
  else
  {
    scaleValuesByRange(value_14bit, knob.MAX_B, knob.MIN_B, &LSB);
    scaleValuesByRange(prev_value_14bit, knob.MAX_B, knob.MIN_B, &oldLSB);
    invertValue(knob.PROPERTIES, INVERT_A_PROPERTY, knob.MAX_A, knob.MIN_A, &MSB);
    invertValue(knob.PROPERTIES, INVERT_B_PROPERTY, knob.MAX_B, knob.MIN_B, &LSB);
    invertValue(knob.PROPERTIES, INVERT_A_PROPERTY, knob.MAX_A, knob.MIN_A, &oldMSB);
    invertValue(knob.PROPERTIES, INVERT_B_PROPERTY, knob.MAX_B, knob.MIN_B, &oldLSB);
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
      isMidiChanged = true;
    }
    if (oldLSB != LSB)
    {
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
        sendNrpnMidiMessage(MIDICoreSerial, knob.MSB, knob.LSB, MSB, LSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB ||
          macro_a_output == OUTPUT_BOTH)
      {
        sendNrpnMidiMessage(MIDICoreUSB, knob.MSB, knob.LSB, MSB, LSB, channel_a);
      }
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_RPN:
    if (oldLSB != LSB)
    {
      if (macro_a_output == OUTPUT_TRS || macro_a_output == OUTPUT_BOTH)
      {
        sendRpnMidiMessage(MIDICoreSerial, knob.MSB, knob.LSB, MSB, LSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB || macro_a_output == OUTPUT_BOTH)
      {
        sendRpnMidiMessage(MIDICoreUSB, knob.MSB, knob.LSB, MSB, LSB, channel_a);
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

template <typename Transport>
void sendNrpnMidiMessage(midi::MidiInterface<Transport> &MidiInterface, uint8_t &msbNumber, uint8_t &lsbNumber, midi::DataByte &MSB, midi::DataByte &LSB, midi::Channel &channel)
{
  MidiInterface.beginNrpn(msbNumber << 7 | lsbNumber, channel);
  MidiInterface.sendNrpnValue(MSB, LSB, channel);
  MidiInterface.endNrpn(channel);
}

template <typename Transport>
void sendRpnMidiMessage(midi::MidiInterface<Transport> &MidiInterface, uint8_t &msbNumber, uint8_t &lsbNumber, midi::DataByte &MSB, midi::DataByte &LSB, midi::Channel &channel)
{
  MidiInterface.beginRpn(msbNumber << 7 | lsbNumber, channel);
  MidiInterface.sendRpnValue(MSB, LSB, channel);
  MidiInterface.endRpn(channel);
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
  device.globalChannel = direction ? (device.globalChannel % 16) + 1 : (device.globalChannel - 2 + 16) % 16 + 1;

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

void buttonReleaseAction(bool direction)
{
  direction ? isPressingAButton = false : isPressingBButton = false;

  if (millis() - pressedTime < SHORT_PRESS_TIME)
    device.isPresetMode ? changePreset(direction) : changeChannel(direction);
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

void extractMode(uint8_t properties, uint8_t *mode)
{
  *mode = (properties & B11110000) >> 4;
}
void extractChannels(uint8_t data, uint8_t properties, midi::Channel *channel_a, midi::Channel *channel_b)
{
  *channel_a =
      bitRead(properties, USE_OWN_CHANNEL_A_PROPERTY)
          ? ((data & 0xF0) >> 4) + 1
          : device.globalChannel;
  *channel_b =
      bitRead(properties, USE_OWN_CHANNEL_B_PROPERTY)
          ? (data & 0xF) + 1
          : device.globalChannel;
}
void extractOutputs(uint8_t outputs, uint8_t *output_a, uint8_t *output_b)
{
  *output_a = (outputs & 0xC) >> 2;
  *output_b = outputs & 0x3;
}