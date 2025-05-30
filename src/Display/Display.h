/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#pragma once

#ifndef N32Bv3
#include <DigitLedDisplay.h>
#else
#include <TLC59282_Display.h>
#endif

#include <Arduino.h>
#include <vector>
#include <bitset>
#include <definitions.h>

#ifndef N32Bv3
class Display : public DigitLedDisplay
#else
class Display : public TLC59282_Display
#endif
{
public:
  static Display &getInstance()
  {
#ifdef N32Bv3
    static Display instance(SIN, SCLK, LAT, BLANK);
#else
    static Display instance(DIN, CS, CLK);
#endif
    // static Display instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }

  // Delete copy constructor and assignment operator to prevent copying
  Display(Display const &) = delete;
  void operator=(Display const &) = delete;

  void clearDisplay(uint16_t readInterval = 1000);
  void showValue(uint8_t, uint8_t);
  void blinkDot(uint8_t);
  void showChannelNumber(uint8_t);
  void showPresetNumber(uint8_t);
  void showStartUpAnimation();
  void factoryResetAnimation();
  void showSaveMessage();
  void showSynching();
  int8_t getActiveKnobIndex();
  void setActiveKnobIndex(uint8_t index);
  void resetActiveKnobIndex();

private:
#ifndef N32Bv3
  Display(uint8_t DIN, uint8_t CS, uint8_t CLK) : DigitLedDisplay(DIN, CS, CLK){};
#else
  Display(uint8_t SIN, uint8_t SCLK, uint8_t LAT, uint8_t BLANK) : TLC59282_Display(SIN, SCLK, LAT, BLANK){};
#endif

  unsigned long displayOffTimer;
  unsigned long lastUpdateTime;
  int8_t activeKnobIndex = -1;
};