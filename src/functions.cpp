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

    uint16_t fullCurrentValue = pot->getCurrentValue();
    uint16_t fullPreviousValue = pot->getPreviousValue();
    uint8_t MSBSendValue = (fullCurrentValue >> 7) & 0x7F;
    uint8_t LSBSendValue;
    uint8_t oldMSBValue = (fullPreviousValue >> 7) & 0x7F;
    uint8_t oldLSBValue = fullPreviousValue & 0x7F;
    uint8_t mode = extractMode(currentKnob->PROPERTIES);

    midi::Channel channel_a =
        bitRead(currentKnob->PROPERTIES, USE_OWN_CHANNEL_A_PROPERTY)
            ? extractChannel(currentKnob->CHANNELS, CHANNEL_A)
            : device.globalChannel;

    midi::Channel channel_b =
        bitRead(currentKnob->PROPERTIES, USE_OWN_CHANNEL_B_PROPERTY)
            ? extractChannel(currentKnob->CHANNELS, CHANNEL_B)
            : device.globalChannel;

    MSBSendValue = map(MSBSendValue, 0, 127, currentKnob->MIN_A, currentKnob->MAX_A);
    if (bitRead(currentKnob->PROPERTIES, INVERT_A_PROPERTY))
    {
      MSBSendValue = map(MSBSendValue, 0, 127, currentKnob->MAX_A, currentKnob->MIN_A);
    }

    if (extractMode(currentKnob->PROPERTIES) == KNOB_MODE_HIRES)
    {
      // Define the total range based on the knob's min and max settings
      uint16_t totalRange = ((currentKnob->MAX_A - currentKnob->MIN_A + 1) << 7);

      // Scale the 14-bit value to the defined total range
      uint32_t scaledValue = ((uint32_t)fullCurrentValue * totalRange) >> 14;
      if (scaledValue > 16383)
        scaledValue = 16383; // Cap the total range to 14-bit max if it exceeds

      // Calculate the MSB and LSB from the scaled value
      MSBSendValue = (scaledValue >> 7) & 0x7F; // Get the top 7 bits as MSB
      LSBSendValue = scaledValue & 0x7F;        // Get the bottom 7 bits as LSB

      // Adjust MSB based on the knob's MIN_A to ensure it starts from the defined minimum
      MSBSendValue += currentKnob->MIN_A;

      // Invert the values if required by the knob's properties
      if (bitRead(currentKnob->PROPERTIES, INVERT_A_PROPERTY))
      {
        MSBSendValue = currentKnob->MAX_A - (MSBSendValue - currentKnob->MIN_A);
        LSBSendValue = 127 - LSBSendValue;
      }
    }
    else
    {
      LSBSendValue = map(MSBSendValue, 0, 127, currentKnob->MIN_B, currentKnob->MAX_B);
      if (bitRead(currentKnob->PROPERTIES, INVERT_B_PROPERTY))
      {
        LSBSendValue = map(MSBSendValue, 0, 127, currentKnob->MAX_B, currentKnob->MIN_B);
      }
    }

    if (device.isStartupCounter == 1100) // TODO: this is an ugly method to wait for startup reading is done. CHANGE IT!
    {
      switch (mode)
      {
      case KNOB_MODE_STANDARD:
        if (oldMSBValue != MSBSendValue)
        {
          sendCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a);
        }
        break;

      case KNOB_MODE_HIRES:
        if (oldLSBValue != LSBSendValue)
        {
          sendCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a);
        }
        break;

      case KNOB_MODE_MACRO:
        if (oldMSBValue != MSBSendValue)
        {
          sendMacroCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a, channel_b);
        }
        break;

      case KNOB_MODE_NRPN:
        if (oldLSBValue != LSBSendValue)
        {
          sendNRPN(currentKnob, MSBSendValue, LSBSendValue, channel_a);
        }
        break;

      case KNOB_MODE_RPN:
        if (oldLSBValue != LSBSendValue)
        {
          sendRPN(currentKnob, MSBSendValue, LSBSendValue, channel_a);
        }
        break;

      case KNOB_MODE_PROGRAM_CHANGE:
        if (oldMSBValue != MSBSendValue)
        {
          sendProgramChange(MSBSendValue, channel_a);
        }
        break;

      case KNOB_MODE_POLY_AFTER_TOUCH:
        if (oldMSBValue != MSBSendValue)
        {
          sendPolyAfterTouch(currentKnob, MSBSendValue, channel_a);
        }
        break;
      case KNOB_MODE_MONO_AFTER_TOUCH:
        if (oldMSBValue != MSBSendValue)
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
    } else {
      device.isStartupCounter++;
    }

    pot->setPreviousValue();
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
  // n32b_display.blinkDot(1);
  n32b_display.showValue(MSBvalue);
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
  // n32b_display.blinkDot(1);
  n32b_display.showValue(MSBvalue);
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
  // n32b_display.blinkDot(1);
  n32b_display.showValue(MSBvalue);
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
  // n32b_display.blinkDot(1);
  n32b_display.showValue(MSBvalue);
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
  // n32b_display.blinkDot(1);
  n32b_display.showValue(MSBvalue);
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
  // n32b_display.blinkDot(1);
  n32b_display.showValue(MSBvalue);
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