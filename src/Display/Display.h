/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#pragma once

#include <TLC59282_Display.h>

#include <Arduino.h>
#include <vector>
#include <bitset>
#include <definitions.h>

class Display : public TLC59282_Display
{
public:
  static Display &getInstance()
  {
  #ifdef DISPLAY_SPI
    static Display instance(&DISPLAY_SPI, DISP_LAT, DISP_BLANK);
  #else
    static Display instance(DISP_SIN, DISP_SCLK, DISP_LAT, DISP_BLANK);
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
#ifdef DISPLAY_SPI
  Display(SPIClass * pSPI_Intf, uint8_t LAT, uint8_t BLANK) : TLC59282_Display(pSPI_Intf, LAT, BLANK){};
#else
  Display(uint8_t SIN, uint8_t SCLK, uint8_t LAT, uint8_t BLANK) : TLC59282_Display(SIN, SCLK, LAT, BLANK){};
#endif

  unsigned long displayOffTimer;
  unsigned long lastUpdateTime;
  int8_t activeKnobIndex = -1;
};
