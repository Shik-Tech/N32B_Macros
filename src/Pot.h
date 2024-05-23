#ifndef POT_h
#define POT_h

#include <Arduino.h>
// #include "definitions.h"

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
};
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

    // Setters
    void setState(Pot_t::State s) { potData.state = s; }
    void setCurrentValue(uint16_t v) { potData.current_value = v; }
    void setPreviousValue() { potData.previous_value = potData.current_value; }
    void resetReleaseCounter() { potData.release_counter = 0; }
    void increaseReleaseCounter() { potData.release_counter++; }

    // Getters
    Pot_t::State &getState() { return potData.state; }
    uint16_t &getCurrentValue() { return potData.current_value; }
    uint16_t &getPreviousValue() { return potData.previous_value; }
    uint8_t &getReleaseCounter() { return potData.release_counter; }

private:
    Pot_t potData;
};

#endif