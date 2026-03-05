/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2025 SHIK
*/

#include "adcMux.h"

ADC_MUX::ADC_MUX(Pot *potsPtr) : pots(potsPtr) {}

void ADC_MUX::init()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(channels[i], OUTPUT);
    }

    for (uint8_t i = 0; i < 2; i++)
    {
        pinMode(signalPin[i], INPUT);
    }
}

uint16_t ADC_MUX::FixedPoint_EMA(uint16_t nSample, uint16_t nPrevValue, uint8_t nAlphaShift)
{
    return (uint16_t)(nPrevValue + nSample - (nPrevValue >> nAlphaShift));
}

void ADC_MUX::update(uint8_t index, bool force)
{
    constexpr uint8_t kEmaAlphaShift = 3;
    constexpr uint8_t kReleaseCountThreshold = 3;

    setMultiplexer(index);
    delayMicroseconds(50);
    const uint16_t sensorRead = read(index);

    Pot &pot = pots[index];

    // EMA: prev (14-bit>>1) + raw (10-bit) - prev/8; result <<1 then map to 14-bit
    uint16_t filteredValue = FixedPoint_EMA(sensorRead, pot.getPreviousValue() >> 1, kEmaAlphaShift) << 1;

    filteredValue = constrain(map(filteredValue, kAdcMapInMin, kAdcMapInMax, 0, kAdc14BitMax), 0, kAdc14BitMax);
    pot.setCurrentValue(filteredValue);

    uint16_t value_difference = (filteredValue >= pot.getPreviousValue()) ? (filteredValue - pot.getPreviousValue()) : (pot.getPreviousValue() - filteredValue);

    if (pot.getState() == Pot_t::IDLE)
    {

        if (value_difference > threshold_idle_to_motion || force)
        {
            pot.setState(Pot_t::IN_MOTION);
        }
    }
    else if (pot.getState() == Pot_t::IN_MOTION)
    {
        if (value_difference < threshold_motion_to_idle)
        {
            pot.increaseReleaseCounter();
            if (pot.getReleaseCounter() > kReleaseCountThreshold)
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
}

uint8_t ADC_MUX::pinSelector(uint8_t index)
{
    return signalPin[index > 15 ? 1 : 0];
}

uint16_t ADC_MUX::read(uint8_t index)
{
    return analogRead(pinSelector(index));
}

void ADC_MUX::setMultiplexer(uint8_t index)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        digitalWrite(channels[i], bitRead(index, i));
    }
}
