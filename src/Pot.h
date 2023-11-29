#ifndef POT_h
#define POT_h

#include <Arduino.h>

struct Pot
{
    enum State
    {
        IDLE,
        IN_MOTION
    } state;
    uint8_t MSBValue;
    uint8_t LSBValue;
    int current_value;
    int previous_value;
    int release_counter;

    Pot() : state(IDLE),
            MSBValue(0),
            LSBValue(0),
            current_value(0),
            previous_value(0),
            release_counter(0) {}
};

#endif