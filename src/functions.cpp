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
  Pot *pot = &device.pots[index];

  if (pot->getState() == Pot_t::IN_MOTION)
  {
    Knob_t *currentKnob = &device.activePreset.knobInfo[index];
    Pot *pot = &device.pots[index];

    uint8_t oldMSBValue = pot->getMSBValue();
    uint8_t oldLSBValue = pot->getLSBValue();
    pot->setMSBValue();
    pot->setLSBValue();
    uint8_t mode = extractMode(currentKnob->PROPERTIES);

    midi::Channel channel_a =
        bitRead(currentKnob->PROPERTIES, USE_OWN_CHANNEL_A_PROPERTY)
            ? extractChannel(currentKnob->CHANNELS, CHANNEL_A)
            : device.globalChannel;

    midi::Channel channel_b =
        bitRead(currentKnob->PROPERTIES, USE_OWN_CHANNEL_B_PROPERTY)
            ? extractChannel(currentKnob->CHANNELS, CHANNEL_B)
            : device.globalChannel;

    uint8_t MSBSendValue = map(pot->getMSBValue(), 0, 127, currentKnob->MIN_A, currentKnob->MAX_A);
    if (bitRead(currentKnob->PROPERTIES, INVERT_A_PROPERTY))
    {
      MSBSendValue = map(pot->getMSBValue(), 0, 127, currentKnob->MAX_A, currentKnob->MIN_A);
    }

    uint8_t LSBSendValue;

    if (extractMode(currentKnob->PROPERTIES) == KNOB_MODE_HIRES)
    {
      LSBSendValue = pot->getLSBValue();

      uint8_t mappedValue = (LSBSendValue < 16) ? 0 : ((LSBSendValue == 16) ? 16 : (LSBSendValue | 0xF));
      LSBSendValue = bitRead(currentKnob->PROPERTIES, INVERT_A_PROPERTY) ? 127 - mappedValue : mappedValue;
    }
    else
    {
      LSBSendValue = map(pot->getMSBValue(), 0, 127, currentKnob->MIN_B, currentKnob->MAX_B);
      if (bitRead(currentKnob->PROPERTIES, INVERT_B_PROPERTY))
      {
        LSBSendValue = map(pot->getMSBValue(), 0, 127, currentKnob->MAX_B, currentKnob->MIN_B);
      }
    }

    switch (mode)
    {
    case KNOB_MODE_STANDARD:
      if (oldMSBValue != pot->getMSBValue())
      {
        sendCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a);
      }
      break;

    case KNOB_MODE_HIRES:
      if (oldLSBValue != pot->getLSBValue())
      {
        sendCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a);
      }
      break;

    case KNOB_MODE_MACRO:
      if (oldMSBValue != pot->getMSBValue())
      {
        sendMacroCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a, channel_b);
      }
      break;

    case KNOB_MODE_NRPN:
      if (oldLSBValue != pot->getLSBValue())
      {
        sendNRPN(currentKnob, MSBSendValue, LSBSendValue, channel_a);
      }
      break;

    case KNOB_MODE_RPN:
      if (oldLSBValue != pot->getLSBValue())
      {
        sendRPN(currentKnob, MSBSendValue, LSBSendValue, channel_a);
      }
      break;

    case KNOB_MODE_PROGRAM_CHANGE:
      if (oldMSBValue != pot->getMSBValue())
      {
        sendProgramChange(MSBSendValue, channel_a);
      }
      break;

    case KNOB_MODE_POLY_AFTER_TOUCH:
      if (oldMSBValue != pot->getMSBValue())
      {
        sendPolyAfterTouch(currentKnob, MSBSendValue, channel_a);
      }
      break;
    case KNOB_MODE_MONO_AFTER_TOUCH:
      if (oldMSBValue != pot->getMSBValue())
      {
        sendMonoAfterTouch(MSBSendValue, channel_a);
      }
      break;

      //    case KNOB_SYSEX:
      //        if (oldLSBValue != LSBValue)
      //        {
      //            sendSysEx(interface, macro, pot);
      //        }
      //        break;
    }
  }
}

void sendCCMessage(const struct Knob_t *currentKnob, uint8_t MSBvalue, uint8_t LSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendControlChange(currentKnob->MSB, MSBvalue, channel);
    if (extractMode(currentKnob->PROPERTIES) == KNOB_MODE_HIRES)
    {
      MIDICoreSerial.sendControlChange(currentKnob->LSB, LSBvalue, channel);
    }
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendControlChange(currentKnob->MSB, MSBvalue, channel);
    if (extractMode(currentKnob->PROPERTIES) == KNOB_MODE_HIRES)
    {
      MIDICoreUSB.sendControlChange(currentKnob->LSB, LSBvalue, channel);
    }
  }
  // n32b_display.blinkDot(1);
  n32b_display.showValue(MSBvalue);
}

void sendMacroCCMessage(const struct Knob_t *currentKnob, uint8_t MSBvalue, uint8_t LSBvalue, midi::Channel channel_a, midi::Channel channel_b)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendControlChange(currentKnob->MSB, MSBvalue, channel_a);
    MIDICoreSerial.sendControlChange(currentKnob->LSB, LSBvalue, channel_b);
  }

  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendControlChange(currentKnob->MSB, MSBvalue, channel_a);
    MIDICoreUSB.sendControlChange(currentKnob->LSB, LSBvalue, channel_b);
  }
  n32b_display.blinkDot(1);
}

void sendNRPN(const struct Knob_t *currentKnob, uint8_t MSBvalue, uint8_t LSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.beginNrpn(currentKnob->MSB << 7 | currentKnob->LSB, channel);
    MIDICoreSerial.sendNrpnValue(MSBvalue, LSBvalue, channel);
    MIDICoreSerial.endNrpn(channel);
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {

    MIDICoreUSB.beginNrpn(currentKnob->MSB << 7 | currentKnob->LSB, channel);
    MIDICoreUSB.sendNrpnValue(MSBvalue, LSBvalue, channel);
    MIDICoreUSB.endNrpn(channel);
  }
  n32b_display.blinkDot(1);
}

void sendRPN(const struct Knob_t *currentKnob, uint8_t MSBvalue, uint8_t LSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.beginRpn(currentKnob->MSB << 7 | currentKnob->LSB, channel);
    MIDICoreSerial.sendRpnValue(MSBvalue, LSBvalue, channel);
    MIDICoreSerial.endRpn(channel);
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.beginRpn(currentKnob->MSB << 7 | currentKnob->LSB, channel);
    MIDICoreUSB.sendRpnValue(MSBvalue, LSBvalue, channel);
    MIDICoreUSB.endRpn(channel);
  }
  n32b_display.blinkDot(1);
}

void sendProgramChange(uint8_t MSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendProgramChange(MSBvalue, channel);
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendProgramChange(MSBvalue, channel);
  }
  n32b_display.blinkDot(1);
}

void sendPolyAfterTouch(const struct Knob_t *currentKnob, uint8_t MSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendAfterTouch(currentKnob->MSB, MSBvalue, channel);
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendAfterTouch(currentKnob->MSB, MSBvalue, channel);
  }
  n32b_display.blinkDot(1);
}

void sendMonoAfterTouch(uint8_t MSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendAfterTouch(MSBvalue, channel);
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendAfterTouch(MSBvalue, channel);
  }
  n32b_display.blinkDot(1);
}

void changeChannel(const bool &direction)
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

void changePreset(const bool &direction)
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
  return (properties & B01110000) >> MODE_PROPERTY;
}
uint8_t extractChannel(const uint8_t &channels, const bool &isA)
{
  uint8_t outChannel = channels & 0xF;
  if (isA)
  {
    outChannel = (channels & 0xF0) >> 4;
  }
  return outChannel + 1;
}