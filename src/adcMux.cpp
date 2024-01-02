/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2023 SHIK
*/

#include "adcMux.h"

ADC_MUX::ADC_MUX(Pot *potsPtr) : pots(potsPtr) {}

void ADC_MUX::init()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(channels[i], OUTPUT);
    }

    pinMode(MUX_A_SIG, INPUT);
    pinMode(MUX_B_SIG, INPUT);
}

void ADC_MUX::update(const uint8_t &index)
{
    setMultiplexer(index);
    delayMicroseconds(10);
    const uint16_t sensorRead = read(index);
    Pot &pot = pots[index];

    // uint16_t filteredValue = constrain(map(sensorRead, 0, 1023, 0, 1023), 0, 1023);
    uint16_t filteredValue = constrain(sensorRead, 0, 1023);

    pot.current_value = filteredValue;

    int value_difference = abs((int)(pot.current_value - pot.previous_value));
    if (pot.state == Pot::IDLE)
    {
        if (value_difference > threshold_idle_to_motion)
        {
            pot.state = Pot::IN_MOTION;
            pot.previous_value = pot.current_value;
        }
    }
    else if (pot.state == Pot::IN_MOTION)
    {
        if (value_difference < threshold_motion_to_idle)
        {
            pot.release_counter++;
            if (pot.release_counter > 3)
            {
                pot.state = Pot::IDLE;
                pot.release_counter = 0;
            }
        }
        else
        {
            pot.release_counter = 0;
            pot.previous_value = pot.current_value;
        }
    }
}

uint8_t ADC_MUX::pinSelector(const uint8_t &index)
{
    return signalPin[index > 15 ? 1 : 0];
}

uint16_t ADC_MUX::read(const uint8_t &index)
{
    return analogRead(pinSelector(index));
}

void ADC_MUX::setMultiplexer(const uint8_t &index)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        digitalWrite(channels[i], bitRead(index, i));
    }
}