#ifndef POT_h
#define POT_h

#include <Arduino.h>
#include "definitions.h"

struct Pot_t
{
    enum State
    {
        IDLE,
        IN_MOTION
    } state;
    // uint8_t MSBValue;
    // uint8_t LSBValue;
    uint8_t release_counter;
    // uint8_t motion_counter;
    uint16_t current_value;
    uint16_t previous_value;
    // uint16_t previous_threshold_value;
};
class Pot
{
public:
    Pot()
    {
        potData.state = Pot_t::IDLE;
        // potData.MSBValue = 0;
        // potData.LSBValue = 0;
        potData.current_value = 0;
        potData.previous_value = 0;
        potData.release_counter = 0;
        // potData.previous_threshold_value = 0;
        // potData.motion_counter = 0;
    };

    // Setters
    void setState(Pot_t::State s) { potData.state = s; }
    // void setMSBValue() { potData.MSBValue = 0x7f & (calculate14bitValue() >> 7); }
    // void setLSBValue() { potData.LSBValue = 0x7f & calculate14bitValue(); }
    // void setMSBValue() { potData.MSBValue = 0x7f & (potData.current_value >> 7); }
    // void setLSBValue() { potData.LSBValue = 0x7f & potData.current_value; }
    void setCurrentValue(uint16_t v) { potData.current_value = v; }
    void setPreviousValue() { potData.previous_value = potData.current_value; }
    // void setPreviousThresholdValue(uint16_t v) { potData.previous_threshold_value = v; }
    void resetReleaseCounter() { potData.release_counter = 0; }
    // void resetMotionCounter() { potData.motion_counter = 0; }
    void increaseReleaseCounter() { potData.release_counter++; }
    // void increaseMotionCounter() { potData.motion_counter++; }

    // Getters
    Pot_t::State &getState() { return potData.state; }
    // uint8_t &getMSBValue() { return potData.MSBValue; }
    // uint8_t &getLSBValue() { return potData.LSBValue; }
    uint16_t &getCurrentValue() { return potData.current_value; }
    uint16_t &getPreviousValue() { return potData.previous_value; }
    // uint16_t &getPreviousThresholdValue() { return potData.previous_threshold_value; }
    uint8_t &getReleaseCounter() { return potData.release_counter; }
    // uint8_t &getMotionCounter() { return potData.motion_counter; }

private:
    Pot_t potData;

    // uint16_t calculate14bitValue() { return (uint16_t)(potData.current_value) << 4; };
};

#endif