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
#include "definitions.h"

/** 7-segment display singleton for knob values, channel, preset, and status messages. */
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
    // Singleton, instantiated on first use.
    return instance;
  }

  // Delete copy constructor and assignment operator to prevent copying
  Display(Display const &) = delete;
  void operator=(Display const &) = delete;

  /** Auto-clear display after idle. @param readInterval Idle ms before clear. @note Call from main loop only. */
  void clearDisplay(uint16_t readInterval = 1000);
  /** Show value for active knob. */
  void showValue(uint8_t, uint8_t);
  /** Blink decimal point on given digit. */
  void blinkDot(uint8_t);
  /** Show channel number with "C" suffix. */
  void showChannelNumber(uint8_t);
  /** Show preset number with "P" suffix. */
  void showPresetNumber(uint8_t);
  /** Play startup animation. @note Blocks for ~2 s (N32B) or ~1.5 s (N32Bv3). */
  void showStartUpAnimation();
  /** Play factory reset (infinity) animation. @note Blocks for ~3 s. */
  void factoryResetAnimation();
  /** Show "Sv." save message. @note Blocks for ~1.2 s. */
  void showSaveMessage();
  /** Show "Syn" synching message. @note Blocks for 1 s. */
  void showSynching();
  /** Index of knob currently shown, or -1 if none. */
  int8_t getActiveKnobIndex();
  /** Set active knob index. */
  void setActiveKnobIndex(uint8_t index);
  /** Clear active knob index. */
  void resetActiveKnobIndex();

private:
#ifndef N32Bv3
  Display(uint8_t DIN, uint8_t CS, uint8_t CLK) : DigitLedDisplay(DIN, CS, CLK){};
#else
  Display(uint8_t SIN, uint8_t SCLK, uint8_t LAT, uint8_t BLANK) : TLC59282_Display(SIN, SCLK, LAT, BLANK){};
#endif

  unsigned long displayOffTimer;
  int8_t activeKnobIndex = -1;
  bool displayCleared = false;
};