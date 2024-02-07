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

uint16_t ADC_MUX::FixedPoint_EMA(uint16_t nSample, uint16_t nPrevValue, uint8_t nAlphaShift)
{
    return (uint16_t)(nPrevValue + nSample - (nPrevValue >> nAlphaShift));
}

void ADC_MUX::update(const uint8_t &index)
{
    setMultiplexer(index);
    // delayMicroseconds(10);
    const uint16_t sensorRead = read(index);
    Pot &pot = pots[index];
    const uint16_t filteredValue = FixedPoint_EMA(sensorRead, pot.getPreviousValue(), 4);

    // uint16_t filteredValue = constrain(map(sensorRead, 0, 1023, 0, 1023), 0, 1023);
    // uint16_t filteredValue = constrain(sensorRead, 0, 1023);

    pot.setCurrentValue(filteredValue);

    // uint16_t value_difference = abs(static_cast<int>(pot.getCurrentValue()) - static_cast<int>(pot.getPreviousValue()));
    // uint16_t value_difference = abs((int) pot.getCurrentValue() - (int) pot.getPreviousValue());
    uint16_t value_difference = (pot.getCurrentValue() >= pot.getPreviousValue()) ? (pot.getCurrentValue() - pot.getPreviousValue()) : (pot.getPreviousValue() - pot.getCurrentValue());

    // if (index == 0)
    // {
    //     Serial.println(sensorRead);
    //     Serial.println(value_difference);
    //     Serial.println("---------");
    // }

    if (pot.getState() == Pot_t::IDLE)
    {
        if (value_difference > threshold_idle_to_motion)
        {
            pot.setState(Pot_t::IN_MOTION);
        }
    }
    else if (pot.getState() == Pot_t::IN_MOTION)
    {
        if (value_difference < threshold_motion_to_idle)
        {
            pot.increaseReleaseCounter();
            if (pot.getReleaseCounter() > 3)
            {
                pot.setState(Pot_t::IDLE);
                pot.resetReleaseCounter();
            }
        }
        else
        {
            pot.resetReleaseCounter();
        }
    }
    pot.setPreviousValue();
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