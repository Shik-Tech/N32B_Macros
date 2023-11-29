/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2023 SHIK
*/

#include <Arduino.h>
#include "definitions.h"
#include "functions.h"

#ifndef ADC_MUX_h
#define ADC_MUX_h

class ADC_MUX
{
public:
  ADC_MUX(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, Pot *pots);
  void init();
  void update(const uint8_t &index);
  void setMultiplexer(const uint8_t &);
  uint8_t pinSelector(const uint8_t &index);
  uint16_t read(const uint8_t &);
  // void init(const uint8_t &, const uint8_t &, const uint8_t &, const uint8_t &);
  // void setSignalPin(const bool &, const uint8_t &);
  // void update(uint8_t &);
  // void setMultiplexer(uint8_t &);
  // uint16_t read(uint8_t &);

private:
  uint8_t mux_a_sig;
  uint8_t mux_b_sig;
  uint8_t muxS0;
  uint8_t muxS1;
  uint8_t muxS2;
  uint8_t muxS3;
  Pot *pots;
  const uint8_t signalPin[2] = {mux_a_sig, mux_b_sig};
  const uint8_t channels[4] = {muxS0, muxS1, muxS2, muxS3};
  // uint8_t channels[4];
  // uint8_t signalPin[2];
};

#endif