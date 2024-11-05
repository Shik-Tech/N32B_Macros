/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "functions.h"
#include <GlobalComponents/GlobalComponents.h>

void onUsbMessage(const midi::Message<128> &message)
{
  if (message.type != ActiveSensing)
  {
    switch (device.activePreset.thruMode)
    {
    case THRU_USB_USB:
      MIDIUSB.send(message.type, message.data1, message.data2, message.channel);
      break;

    case THRU_USB_TRS:
      MIDISerial1.send(message);
      break;

    case THRU_BOTH_DIRECTIONS:
      MIDIUSB.send(message.type, message.data1, message.data2, message.channel);
      MIDISerial1.send(message);
      break;
    }
    display.blinkDot(2);
  }
}

void onSerialMessage(const midi::Message<128> &message)
{
  if (message.type != ActiveSensing)
  {
    switch (device.activePreset.thruMode)
    {
    case THRU_TRS_TRS:
      MIDISerial1.send(message);
      break;

    case THRU_TRS_USB:
      MIDIUSB.send(message.type, message.data1, message.data2, message.channel);
      break;

    case THRU_BOTH_DIRECTIONS:
      MIDIUSB.send(message.type, message.data1, message.data2, message.channel);
      MIDISerial1.send(message);
      break;
    }
    display.blinkDot(2);
  }
}

void updateKnob(uint8_t &index, bool force)
{
  Pot &pot = device.pots[index];
  Pot::Pot_t::State potState = pot.getState();
  if (potState == Pot::Pot_t::IN_MOTION)
  {
    sendMidiMessage(index, force);
  }

  if (potState == Pot::Pot_t::IDLE && display.getActiveKnobIndex() == index)
  {
    display.resetActiveKnobIndex();
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
  uint16_t range = max - min + 1;
  uint16_t totalRange = range << 7; // 128 steps for each unit in range

  // Scale the 14-bit value to the defined total range
  uint32_t scaledValue = ((uint32_t)value * totalRange) >> 14;

  // Cap the total range to 14-bit max if it exceeds
  if (scaledValue > 16383)
    scaledValue = 16383;

  // Calculate the MSB or LSB from the scaled value
  if (isLSB)
  {
    *outputValue = scaledValue & 0x7F; // LSB ranges from 0-127 within each MSB step
  }
  else
  {
    *outputValue = (scaledValue >> 7) & 0x7F; // Get the top 7 bits as MSB
    *outputValue += min;                      // Adjust the MSB to start from the defined minimum
  }
}

void sendMidiMessage(uint8_t &index, bool force)
{
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
    scaleValuesByRange(value_14bit, knob.MAX_A, knob.MIN_A, &LSB, true);
    scaleValuesByRange(prev_value_14bit, knob.MAX_A, knob.MIN_A, &oldLSB, true);
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
    if (force || oldMSB != MSB)
    {
      sendStandardCCMessage(macro_a_output, knob.MSB, MSB, channel_a);
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_HIRES:
    if (force || oldLSB != LSB)
    {
      sendStandardCCMessage(macro_a_output, knob.MSB, MSB, channel_a);
      sendStandardCCMessage(macro_a_output, knob.LSB, LSB, channel_a);

      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_MACRO:
    if (force || oldMSB != MSB)
    {
      sendStandardCCMessage(macro_a_output, knob.MSB, MSB, channel_a);
      isMidiChanged = true;
    }
    if (force || oldLSB != LSB)
    {
      sendStandardCCMessage(macro_b_output, knob.LSB, LSB, channel_b);
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_NRPN:
    if (force || oldLSB != LSB)
    {
      if (macro_a_output == OUTPUT_TRS ||
          macro_a_output == OUTPUT_BOTH)
      {
        sendNrpnMidiMessage(MIDISerial1, knob.MSB, knob.LSB, MSB, LSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB ||
          macro_a_output == OUTPUT_BOTH)
      {
        sendNrpnMidiMessage(MIDIUSB, knob.MSB, knob.LSB, MSB, LSB, channel_a);
      }
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_RPN:
    if (force || oldLSB != LSB)
    {
      if (macro_a_output == OUTPUT_TRS || macro_a_output == OUTPUT_BOTH)
      {
        sendRpnMidiMessage(MIDISerial1, knob.MSB, knob.LSB, MSB, LSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB || macro_a_output == OUTPUT_BOTH)
      {
        sendRpnMidiMessage(MIDIUSB, knob.MSB, knob.LSB, MSB, LSB, channel_a);
      }
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_PROGRAM_CHANGE:
    if (force || oldMSB != MSB)
    {
      if (macro_a_output == OUTPUT_TRS || macro_a_output == OUTPUT_BOTH)
      {
        MIDISerial1.sendProgramChange(MSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB || macro_a_output == OUTPUT_BOTH)
      {
        MIDIUSB.sendProgramChange(MSB, channel_a);
      }

      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_POLY_AFTER_TOUCH:
    if (force || oldMSB != MSB)
    {
      if (macro_a_output == OUTPUT_TRS || macro_a_output == OUTPUT_BOTH)
      {
        MIDISerial1.sendAfterTouch(knob.MSB, MSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB || macro_a_output == OUTPUT_BOTH)
      {
        MIDIUSB.sendAfterTouch(knob.MSB, MSB, channel_a);
      }
      isMidiChanged = true;
    }
    break;
  case KNOB_MODE_MONO_AFTER_TOUCH:
    if (force || oldMSB != MSB)
    {
      if (macro_a_output == OUTPUT_TRS || macro_a_output == OUTPUT_BOTH)
      {
        MIDISerial1.sendAfterTouch(MSB, channel_a);
      }
      if (macro_a_output == OUTPUT_USB || macro_a_output == OUTPUT_BOTH)
      {
        MIDIUSB.sendAfterTouch(MSB, channel_a);
      }
      isMidiChanged = true;
    }
    break;
  }

  if (isMidiChanged)
  {
    display.showValue(MSB, index);
  }
}

template <typename Transport>
void sendNrpnMidiMessage(midi::MidiInterface<Transport, CustomMidiSettings> &MidiInterface, uint8_t &msbNumber, uint8_t &lsbNumber, midi::DataByte &MSB, midi::DataByte &LSB, midi::Channel &channel)
{
  MidiInterface.beginNrpn(msbNumber << 7 | lsbNumber, channel);
  MidiInterface.sendNrpnValue(MSB, LSB, channel);
  MidiInterface.endNrpn(channel);
}

template <typename Transport>
void sendRpnMidiMessage(midi::MidiInterface<Transport, CustomMidiSettings> &MidiInterface, uint8_t &msbNumber, uint8_t &lsbNumber, midi::DataByte &MSB, midi::DataByte &LSB, midi::Channel &channel)
{
  MidiInterface.beginRpn(msbNumber << 7 | lsbNumber, channel);
  MidiInterface.sendRpnValue(MSB, LSB, channel);
  MidiInterface.endRpn(channel);
}

void sendStandardCCMessage(uint8_t output, uint8_t message, uint8_t value, midi::Channel channel)
{
  if (output == OUTPUT_TRS || output == OUTPUT_BOTH)
  {
    MIDISerial1.sendControlChange(message, value, channel);
  }
  if (output == OUTPUT_USB || output == OUTPUT_BOTH)
  {
    MIDIUSB.sendControlChange(message, value, channel);
  }
}

void changeChannel(bool direction)
{
  device.globalChannel = direction ? (device.globalChannel % 16) + 1 : (device.globalChannel - 2 + 16) % 16 + 1;

  display.showChannelNumber(device.globalChannel);
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

void sendSnapshot()
{
  display.showSynching();
  /*
  for (uint8_t currentKnob = 0; currentKnob < NUMBER_OF_KNOBS; currentKnob++)
  {
    muxFactory.update(currentKnob, true);
  }
  */
  for (uint8_t currentKnob = 0; currentKnob < NUMBER_OF_KNOBS; currentKnob++)
  {
    updateKnob(currentKnob, true);
  }
}

void handleButtons()
{
  // Read button states
  bool buttonA = digitalRead(BUTTON_A) == LOW;
  bool buttonB = digitalRead(BUTTON_B) == LOW;
  unsigned long currentTime = millis();

  // Handle button A
  if (buttonA)
  {
    if (device.buttonAState == BUTTON_IDLE)
    {
      device.buttonAState = BUTTON_PRESSED;
      device.buttonAPressTime = currentTime;
    }
    else if (device.buttonAState == BUTTON_PRESSED && (currentTime - device.buttonAPressTime > LONG_PRESS_THRESHOLD))
    {
      device.buttonAState = BUTTON_LONG_PRESSED;
      if (device.currentMode == PRESET_SELECT)
      {
        device.currentMode = CHANNEL_SELECT;
        display.showChannelNumber(device.globalChannel);
      }
    }
  }
  else
  {
    if (device.buttonAState == BUTTON_PRESSED)
    {
      // Short press detected on release
      if (device.currentMode == CHANNEL_SELECT)
      {
        changeChannel(1); // Change channel up
      }
      else if (device.currentMode == PRESET_SELECT)
      {
        changePreset(1); // Change preset up
      }
    }
    device.buttonAState = BUTTON_IDLE;
  }

  // Handle button B
  if (buttonB)
  {
    if (device.buttonBState == BUTTON_IDLE)
    {
      device.buttonBState = BUTTON_PRESSED;
      device.buttonBPressTime = currentTime;
    }
    else if (device.buttonBState == BUTTON_PRESSED && (currentTime - device.buttonBPressTime > LONG_PRESS_THRESHOLD))
    {
      device.buttonBState = BUTTON_LONG_PRESSED;
      if (device.currentMode == CHANNEL_SELECT)
      {
        device.currentMode = PRESET_SELECT;
        display.showPresetNumber(device.currentPresetIndex);
      }
      else if (device.currentMode == PRESET_SELECT)
      {
        sendSnapshot();
      }
    }
  }
  else
  {
    if (device.buttonBState == BUTTON_PRESSED)
    {
      // Short press detected on release
      if (device.currentMode == CHANNEL_SELECT)
      {
        changeChannel(0); // Change channel down
      }
      else if (device.currentMode == PRESET_SELECT)
      {
        changePreset(0); // Change preset down
      }
    }
    device.buttonBState = BUTTON_IDLE;
  }
}

// void doMidiRead()
// {
//   MIDISerial1.read();
//   MIDIUSB.read();
// }

void extractMode(uint8_t properties, uint8_t *mode)
{
  *mode = (properties & 0b11110000) >> 4;
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
