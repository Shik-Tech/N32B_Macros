/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
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

#ifndef N32Bv3
class N32B_DISPLAY : public DigitLedDisplay
#else
class N32B_DISPLAY : public TLC59282_Display
#endif
{
public:
#ifndef N32Bv3
  N32B_DISPLAY(uint8_t DIN, uint8_t CS, uint8_t CLK) : DigitLedDisplay(DIN, CS, CLK){};
#else
  N32B_DISPLAY(uint8_t SIN, uint8_t SCLK, uint8_t LAT, uint8_t BLANK) : TLC59282_Display(SIN, SCLK, LAT, BLANK){};
#endif

  void clearDisplay(uint16_t readInterval = 1000);
  void showValue(uint8_t);
  void blinkDot(uint8_t);
  void showChannelNumber(uint8_t);
  void showPresetNumber(uint8_t);
  void showStartUpAnimation();
  void factoryResetAnimation();
  void showSaveMessage();
  void showSynching();

private:
  unsigned long displayOffTimer;
  unsigned long lastUpdateTime;
};