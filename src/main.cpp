/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "functions.h"
#include "devices.h"

void setup()
{
#ifndef N32Bv3
  n32b_display.setBright(0);
  n32b_display.setDigitLimit(2);
#else
  n32b_display.on();
#endif

  /* Pin setup */
  pinMode(MIDI_TX_PIN, OUTPUT);
  digitalWrite(MIDI_TX_PIN, HIGH); // Prevent random messages on startup
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);

  /*
   * Factory Reset
   * Hold button-A down while powering the device will reset the presets
   */
  if (!digitalRead(BUTTON_A_PIN))
  {
    bool buttonPressed = true;
    digitalWrite(LED_PIN, LOW);

    while (millis() < reset_timeout) // Check if button has been released before timeout
    {
      if (digitalRead(BUTTON_A_PIN))
      {
        buttonPressed = false;
        break;
      }
    }

    // If button is still held down, then clear eeprom
    if (buttonPressed)
    {
      // Blink once if reset request has been accepted
      digitalWrite(LED_PIN, HIGH);
      delay(20);
      digitalWrite(LED_PIN, LOW);

      // Clean eeprom
      for (unsigned int i = 0; i < EEPROM.length(); i++)
      {
        EEPROM.write(i, 0);
      }
      digitalWrite(LED_PIN, HIGH);
    }
  }

  // Write the factory presets to memory if the device was turn on for the first time
  if (!isEEPROMvalid())
  {
    for (int i = 0; i < NUMBER_OF_PRESETS; i++)
    {
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
    }
    n32b_display.factoryResetAnimation();
    formatFactory();
  }

  muxFactory.init();

  // Load the last used preset as stored in EEPROM
  loadPreset(EEPROM.read(lastUsedPresetAddress));

  /* Set callbacks */
  MIDICoreUSB.setHandleMessage(onUsbMessage);
  MIDICoreSerial.setHandleMessage(onSerialMessage);

  MIDICoreUSB.setHandleSystemExclusive(processSysex);
  MIDICoreSerial.setHandleSystemExclusive(processSysex);

  MIDICoreUSB.setHandleProgramChange(handleProgramChange);
  MIDICoreSerial.setHandleProgramChange(handleProgramChange);

  /* Initiate MIDI communications, listen to all channels */
  MIDICoreUSB.begin(MIDI_CHANNEL_OMNI);
  MIDICoreSerial.begin(MIDI_CHANNEL_OMNI);

  n32b_display.showStartUpAnimation();
}

void loop()
{
  for (uint8_t currentKnob = 0; currentKnob < NUMBER_OF_KNOBS; currentKnob++)
  {
    muxFactory.update(currentKnob);
  }

#ifdef N32Bv3
  n32b_display.resetChanged();
#endif

  for (uint8_t currentKnob = 0; currentKnob < NUMBER_OF_KNOBS; currentKnob++)
  {
    updateKnob(currentKnob);
  }

  doMidiRead();

  handleButtons();
  n32b_display.clearDisplay();

#ifdef N32Bv3
  if (n32b_display.hasChanged())
    delayMicroseconds(1000);
#endif
}
