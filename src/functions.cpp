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

void updateKnob(const uint8_t &index)
{
  Pot *pot = &pots[index];
  if (pot->state == Pot::IN_MOTION)
  {
    const uint16_t value_14bit = (uint16_t)(pot->current_value) << 4;
    sendMacroCCMessage(index, value_14bit);
  }
}

void sendMacroCCMessage(const uint8_t &index, const uint16_t &value_14bit)
{
  Knob_t *currentKnob = &device.activePreset.knobInfo[index];
  Pot *pot = &pots[index];

  uint8_t oldMSBValue = pot->MSBValue;
  uint8_t oldLSBValue = pot->LSBValue;
  pot->MSBValue = 0x7f & (value_14bit >> 7);
  pot->LSBValue = 0x7f & value_14bit;
  uint8_t &MSBValue = pot->MSBValue;
  uint8_t &LSBValue = pot->LSBValue;
  bool isMidiChanged = false;

  uint8_t mode = extractMode(currentKnob->PROPERTIES);
  midi::Channel channel_a =
      bitRead(currentKnob->PROPERTIES, USE_OWN_CHANNEL_A_PROPERTY)
          ? extractChannel(currentKnob->CHANNELS, CHANNEL_A)
          : device.globalChannel;

  midi::Channel channel_b =
      bitRead(currentKnob->PROPERTIES, USE_OWN_CHANNEL_B_PROPERTY)
          ? extractChannel(currentKnob->CHANNELS, CHANNEL_B)
          : device.globalChannel;

  uint8_t MSBSendValue = map(MSBValue, 0, 127, currentKnob->MIN_A, currentKnob->MAX_A);
  if (bitRead(currentKnob->PROPERTIES, INVERT_A_PROPERTY))
  {
    MSBSendValue = map(MSBValue, 0, 127, currentKnob->MAX_A, currentKnob->MIN_A);
  }

  uint8_t LSBSendValue;
  if (extractMode(currentKnob->PROPERTIES) == KNOB_MODE_HIRES)
  {
    LSBSendValue = LSBValue;
    if (bitRead(currentKnob->PROPERTIES, INVERT_A_PROPERTY))
    {
      LSBSendValue = 127 - LSBValue;
    }
  }
  else
  {
    LSBSendValue = map(MSBValue, 0, 127, currentKnob->MIN_B, currentKnob->MAX_B);
    if (bitRead(currentKnob->PROPERTIES, INVERT_B_PROPERTY))
    {
      LSBSendValue = map(MSBValue, 0, 127, currentKnob->MAX_B, currentKnob->MIN_B);
    }
  }

  switch (mode)
  {
  case KNOB_MODE_STANDARD:
    if (oldMSBValue != MSBValue)
    {
      sendCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a);

      // sendMacroCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a, channel_b);
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_HIRES:
    if (oldLSBValue != LSBValue)
    {
      sendCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a);
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_NRPN:
    if (oldLSBValue != LSBValue)
    {
      sendNRPM(currentKnob, MSBSendValue, channel_a);

      // interface.beginNrpn(currentKnob->MSB << 7 | macro->lsb, channel);
      // interface.sendNrpnValue(MSBValue, LSBValue, channel);
      // interface.endNrpn(channel);
      isMidiChanged = true;
    }
    break;

  case KNOB_MODE_RPN:
    if (oldLSBValue != LSBValue)
    {
      sendRPM(currentKnob, MSBSendValue, channel_a);

      // interface.beginRpn(currentKnob->MSB << 7 | macro->lsb, channel);
      // interface.sendRpnValue(MSBValue, LSBValue, channel);
      // interface.endRpn(channel);
      isMidiChanged = true;
    }
    break;

  // case KNOB_MODE_PROGRAM_CHANGE:
  //   if (oldMSBValue != MSBValue)
  //   {
  //     interface.sendProgramChange(MSBValue, channel);
  //     isMidiChanged = true;
  //   }
  //   break;

  // case KNOB_MODE_AFTER_TOUCH:
  //   if (oldMSBValue != MSBValue)
  //   {
  //     interface.sendAfterTouch(MSBValue, channel);
  //     isMidiChanged = true;
  //   }
  //   break;

    //    case KNOB_SYSEX:
    //        if (oldLSBValue != LSBValue)
    //        {
    //            sendSysEx(interface, macro, pot);
    //        }
    //        break;
  }

  ///

  // for (uint8_t macroIndex = 0; macroIndex < NUMBER_OF_MACROS; macroIndex++)
  // {
  //   const Macro_t *macro = &currentKnob->macros[macroIndex];
  //   if (macro->status)
  //   {
  //     const midi::Channel channel = macro->channel ? macro->channel : globalSettings.globalChannel + 1;
  //     calculateMSBValue(macro, oldMSBValue);
  //     calculateMSBValue(macro, MSBValue);
  //     calculateLSBValue(macro, oldLSBValue);
  //     calculateLSBValue(macro, LSBValue);

  //     for (uint8_t interfaceIndex = 0; interfaceIndex < 3; interfaceIndex++)
  //     {
  //       if (bitRead(macro->activeMidiOutputs, interfaceIndex))
  //       {
  //         auto &midiInterface = midiInterfaces[interfaceIndex];

  //         std::visit([&](auto &&interface)
  //                    {
  //                      switch (macro->mode)
  //                      {
  //                      case KNOB_MODE_STANDARD:
  //                        if (oldMSBValue != MSBValue)
  //                        {
  //                          interface.sendControlChange(macro->msb, MSBValue, channel);
  //                          isMidiChanged = true;
  //                        }
  //                        break;

  //                      case KNOB_MODE_HIRES:
  //                        if (oldLSBValue != LSBValue)
  //                        {
  //                          interface.sendControlChange(macro->msb, MSBValue, channel);
  //                          interface.sendControlChange(macro->lsb, LSBValue, channel);
  //                          isMidiChanged = true;
  //                        }
  //                        break;

  //                      case KNOB_MODE_NRPN:
  //                        if (oldLSBValue != LSBValue)
  //                        {
  //                          interface.beginNrpn(macro->msb << 7 | macro->lsb, channel);
  //                          interface.sendNrpnValue(MSBValue, LSBValue, channel);
  //                          interface.endNrpn(channel);
  //                          isMidiChanged = true;
  //                        }
  //                        break;

  //                      case KNOB_MODE_RPN:
  //                        if (oldLSBValue != LSBValue)
  //                        {
  //                          interface.beginRpn(macro->msb << 7 | macro->lsb, channel);
  //                          interface.sendRpnValue(MSBValue, LSBValue, channel);
  //                          interface.endRpn(channel);
  //                          isMidiChanged = true;
  //                        }
  //                        break;

  //                      case KNOB_MODE_PROGRAM_CHANGE:
  //                        if (oldMSBValue != MSBValue)
  //                        {
  //                          interface.sendProgramChange(MSBValue, channel);
  //                          isMidiChanged = true;
  //                        }
  //                        break;

  //                      case KNOB_MODE_AFTER_TOUCH:
  //                        if (oldMSBValue != MSBValue)
  //                        {
  //                          interface.sendAfterTouch(MSBValue, channel);
  //                          isMidiChanged = true;
  //                        }
  //                        break;

  //                        //    case KNOB_SYSEX:
  //                        //        if (oldLSBValue != LSBValue)
  //                        //        {
  //                        //            sendSysEx(interface, macro, pot);
  //                        //        }
  //                        //        break;
  //                      }

  //                      //
  //                    },
  //                    midiInterface);

  //         if (interfaceIndex != 0 && isMidiChanged)
  //         {
  //           const uint8_t &currentScreen = menuNavigation.getCurrentScreen();
  //           display.updateKnobValue(index, currentScreen);
  //           leds.turnLedOn(interfaceIndex);
  //         }
  //       }
  //     }
  //   }
  // }
}

// void updateKnob(uint8_t index)
// {
//   Knob_t &currentKnob = device.activePreset.knobInfo[index];
//   bool needToUpdate = false;
//   uint16_t shiftedValue;
//   uint16_t oldShiftedValue;
//   uint8_t MSBValue;
//   uint8_t oldMSBValue;
//   uint8_t LSBValue;
//   uint8_t oldLSBValue;

//   shiftedValue = map(device.knobValues[index][0], 0, 1023, 0, 16383);
//   oldShiftedValue = map(device.knobValues[index][2], 0, 1023, 0, 16383);
//   MSBValue = shiftedValue >> 7;
//   oldMSBValue = oldShiftedValue >> 7;
//   LSBValue = lowByte(shiftedValue) >> 1;
//   oldLSBValue = lowByte(oldShiftedValue) >> 1;

//   if (extractMode(currentKnob.PROPERTIES) == KNOB_MODE_HIRES)
//   {
//     if ((device.knobValues[index][0] != device.knobValues[index][2]) && LSBValue != oldLSBValue)
//     {
//       needToUpdate = true;
//     }
//   }
//   else
//   {
//     if ((device.knobValues[index][0] != device.knobValues[index][2]) && MSBValue != oldMSBValue)
//     {
//       needToUpdate = true;
//     }
//   }

//   if (needToUpdate)
//   {
//     device.knobValues[index][2] = device.knobValues[index][0];

//     uint8_t mode = extractMode(currentKnob.PROPERTIES);
//     midi::Channel channel_a =
//         bitRead(currentKnob.PROPERTIES, USE_OWN_CHANNEL_A_PROPERTY)
//             ? extractChannel(currentKnob.CHANNELS, CHANNEL_A)
//             : device.globalChannel;

//     midi::Channel channel_b =
//         bitRead(currentKnob.PROPERTIES, USE_OWN_CHANNEL_B_PROPERTY)
//             ? extractChannel(currentKnob.CHANNELS, CHANNEL_B)
//             : device.globalChannel;

//     uint8_t MSBSendValue = map(MSBValue, 0, 127, currentKnob.MIN_A, currentKnob.MAX_A);
//     if (bitRead(currentKnob.PROPERTIES, INVERT_A_PROPERTY))
//     {
//       MSBSendValue = map(MSBValue, 0, 127, currentKnob.MAX_A, currentKnob.MIN_A);
//     }

//     uint8_t LSBSendValue;
//     if (extractMode(currentKnob.PROPERTIES) == KNOB_MODE_HIRES)
//     {
//       LSBSendValue = LSBValue;
//       if (bitRead(currentKnob.PROPERTIES, INVERT_A_PROPERTY))
//       {
//         LSBSendValue = 127 - LSBValue;
//       }
//     }
//     else
//     {
//       LSBSendValue = map(MSBValue, 0, 127, currentKnob.MIN_B, currentKnob.MAX_B);
//       if (bitRead(currentKnob.PROPERTIES, INVERT_B_PROPERTY))
//       {
//         LSBSendValue = map(MSBValue, 0, 127, currentKnob.MAX_B, currentKnob.MIN_B);
//       }
//     }

//     switch (mode)
//     {
//     case KNOB_MODE_STANDARD:
//     case KNOB_MODE_HIRES:
//       sendCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a);
//       break;

//     case KNOB_MODE_MACRO:
//       sendMacroCCMessage(currentKnob, MSBSendValue, LSBSendValue, channel_a, channel_b);
//       break;

//     case KNOB_MODE_NRPN:
//       sendNRPM(currentKnob, MSBSendValue, channel_a);
//       break;

//     case KNOB_MODE_RPN:
//       sendRPM(currentKnob, MSBSendValue, channel_a);
//       break;

//     default:
//       break;
//     }
//   }
// }

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

void sendNRPM(const struct Knob_t *currentKnob, uint8_t MSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendControlChange(99, currentKnob->MSB & 0x7F, channel); // NRPN MSB
    MIDICoreSerial.sendControlChange(98, currentKnob->LSB & 0x7F, channel); // NRPN LSB
    MIDICoreSerial.sendControlChange(6, MSBvalue, channel);                // Data Entry MSB
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendControlChange(99, currentKnob->MSB & 0x7F, channel); // NRPN MSB
    MIDICoreUSB.sendControlChange(98, currentKnob->LSB & 0x7F, channel); // NRPN LSB
    MIDICoreUSB.sendControlChange(6, MSBvalue, channel);                // Data Entry MSB
  }
  n32b_display.blinkDot(1);
}

void sendRPM(const struct Knob_t *currentKnob, uint8_t MSBvalue, midi::Channel channel)
{
  if (device.activePreset.outputMode == OUTPUT_TRS ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreSerial.sendControlChange(101, currentKnob->MSB & 0x7F, channel); // RPN MSB
    MIDICoreSerial.sendControlChange(100, currentKnob->LSB & 0x7F, channel); // RPN LSB
    MIDICoreSerial.sendControlChange(6, MSBvalue, channel);                 // Data Entry MSB
  }
  if (device.activePreset.outputMode == OUTPUT_USB ||
      device.activePreset.outputMode == OUTPUT_BOTH)
  {
    MIDICoreUSB.sendControlChange(101, currentKnob->MSB & 0x7F, channel); // RPN MSB
    MIDICoreUSB.sendControlChange(100, currentKnob->LSB & 0x7F, channel); // RPN LSB
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