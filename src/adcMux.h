/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#ifndef ADC_MUX_h
#define ADC_MUX_h

#include <Arduino.h>
#include "definitions.h"
#include "functions.h"

class ADC_MUX
{
public:
  ADC_MUX(Pot *pots);
  void init();
  void update(const uint8_t &index);
  void setMultiplexer(const uint8_t &);
  uint8_t pinSelector(const uint8_t &index);
  uint16_t read(const uint8_t &);

private:
  Pot *pots;
  const uint8_t signalPin[2] = {MUX_A_SIG, MUX_B_SIG};
  const uint8_t channels[4] = {MUX_S0, MUX_S1, MUX_S2, MUX_S3};
};

#endif