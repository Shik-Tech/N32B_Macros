/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#ifndef ADC_MUX_h
#define ADC_MUX_h

#include <vector>
#include "definitions.h"
#include "CustomADC.h"
#include "Pot.h"
#include "ControlEventBuffer.h"

constexpr uint8_t NUMBER_OF_CONTROLS = NUMBER_OF_KNOBS;

class ADC_MUX
{
public:
  ADC_MUX(
	std::array<Pot, NUMBER_OF_CONTROLS> &potsRef, CustomADC &customADC,
	ControlEventBuffer<CONTROL_EVENT_BUFFER_SIZE> &controlEventBuffer);

  void update(uint16_t sample);
  void setMultiplexer(const uint8_t &);
  uint8_t pinSelector(const uint8_t &index);
  void triggerADC(const uint8_t &);

private:
  const uint8_t signalPin[2] = {AN_MUX_OUT1, AN_MUX_OUT2};
  const uint8_t channels[4] = {AN_MUX_S0, AN_MUX_S1, AN_MUX_S2, AN_MUX_S3};
  std::array<Pot, NUMBER_OF_CONTROLS> &pots;
  
  static constexpr uint16_t muxSettlingDelay = 5; // µs
  
  CustomADC &adc;
  ControlEventBuffer<CONTROL_EVENT_BUFFER_SIZE> &controlEventBuffer;
  
  static constexpr uint8_t invalidConvChannelIndex = NUMBER_OF_CONTROLS;
  uint8_t curConvChannelIndex = invalidConvChannelIndex;

  static inline uint16_t FixedPoint_EMA(uint16_t nSample, uint16_t nPrevValue, uint8_t nAlphaShift);
};

#endif
