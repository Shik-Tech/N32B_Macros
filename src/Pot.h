/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#pragma once

#include <Arduino.h>

/** Pot state and value storage for ADC multiplexer. */
struct Pot_t
{
    /** Idle vs in-motion state for threshold and debounce logic. */
    enum State
    {
        IDLE,
        IN_MOTION
    } state;
    /** Debounce counter for idle transition (in-motion state machine). */
    uint8_t release_counter;
    uint16_t current_value;
    uint16_t previous_value;
};

/** Wrapper for pot state machine (idle/in-motion, EMA-filtered values). */
class Pot
{
public:
    Pot()
    {
        potData.state = Pot_t::IDLE;
        potData.current_value = 0;
        potData.previous_value = 0;
        potData.release_counter = 0;
    };

    /** Sets pot state (IDLE or IN_MOTION). */
    void setState(Pot_t::State s) { potData.state = s; }
    /** Sets current ADC value. */
    void setCurrentValue(uint16_t v) { potData.current_value = v; }
    /** Copies current value to previous (for delta detection). */
    void setPreviousValue() { potData.previous_value = potData.current_value; }
    /** Resets release debounce counter. */
    void resetReleaseCounter() { potData.release_counter = 0; }
    /** Increments release debounce counter. */
    void increaseReleaseCounter() { potData.release_counter++; }

    /** Returns pot state. */
    Pot_t::State getState() const { return potData.state; }
    /** Returns current ADC value. */
    uint16_t getCurrentValue() const { return potData.current_value; }
    /** Returns previous ADC value. */
    uint16_t getPreviousValue() const { return potData.previous_value; }
    /** Returns release debounce counter. */
    uint8_t getReleaseCounter() const { return potData.release_counter; }

private:
    Pot_t potData;
};
