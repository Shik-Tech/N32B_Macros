/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#pragma once

#include <Arduino.h>

class Pot
{
public:
    struct Pot_t
    {
        enum State
        {
            IDLE,
            IN_MOTION
        } state;
        uint8_t release_counter;
        uint16_t current_value;
        uint16_t previous_value;
        uint16_t previous_value_EMA;
        uint16_t previous_value_filtered;
    };
    
    Pot()
    {
        potData.state = Pot_t::IDLE;
        potData.current_value = 0;
        potData.previous_value = 0;
        potData.release_counter = 0;
        potData.previous_value_EMA = 0;
        potData.previous_value_filtered = 0;
    };

    // Setters
    void setState(Pot_t::State s) { potData.state = s; }
    void setCurrentValue(uint16_t v) { potData.current_value = v; }
    //void setPreviousValue() { potData.previous_value = potData.current_value; }
    void setPreviousValue(uint16_t v) { potData.previous_value = v; }
    void setPreviousValue_EMA(uint16_t v) { potData.previous_value_EMA = v; }
    void setPreviousValue_Filtered(uint16_t v) { potData.previous_value_filtered = v; }
    void resetReleaseCounter() { potData.release_counter = 0; }
    void increaseReleaseCounter() { potData.release_counter++; }

    // Getters
    Pot_t::State &getState() { return potData.state; }
    uint16_t &getCurrentValue() { return potData.current_value; }
    uint16_t &getPreviousValue() { return potData.previous_value; }
    uint16_t &getPreviousValue_EMA() { return potData.previous_value_EMA; }
    uint16_t &getPreviousValue_Filtered() { return potData.previous_value_filtered; }
    uint8_t &getReleaseCounter() { return potData.release_counter; }

private:
    Pot_t potData;
};
