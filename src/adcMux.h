/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#pragma once

#include <Arduino.h>
#include "definitions.h"
#include "Pot.h"

/** 32-channel ADC multiplexer for pots via two 16-channel muxes. */
class ADC_MUX
{
public:
  /** @param potsPtr Array of 32 pots. */
  ADC_MUX(Pot *potsPtr);

  /** Configures mux select and signal pins.
   * @note Must be called before update().
   */
  void init();

  /** Reads ADC, applies EMA filter, updates pot state machine.
   * @param index Knob index 0–31.
   * @param force Bypass idle threshold (e.g. for snapshot sync).
   * @note Call from main loop only.
   */
  void update(uint8_t index, bool force = false);

  /** Selects mux channel for given knob.
   * @param index Knob index 0–31.
   */
  void setMultiplexer(uint8_t index);

  /** Returns analog pin (MUX_A_SIG or MUX_B_SIG) for given knob.
   * @param index Knob index 0–31.
   * @return Analog pin number.
   */
  uint8_t pinSelector(uint8_t index);

  /** Reads 10-bit ADC value for given knob.
   * @param index Knob index 0–31.
   * @return 10-bit value (0–1023).
   */
  uint16_t read(uint8_t index);

private:
  Pot *pots;
  const uint8_t signalPin[2] = {MUX_A_SIG, MUX_B_SIG};
  const uint8_t channels[4] = {MUX_S0, MUX_S1, MUX_S2, MUX_S3};

  /** Fixed-point EMA: prev + sample - (prev >> alpha). */
  static inline uint16_t FixedPoint_EMA(uint16_t nSample, uint16_t nPrevValue, uint8_t nAlphaShift);
};
