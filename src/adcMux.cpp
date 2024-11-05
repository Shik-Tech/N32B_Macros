/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "adcMux.h"

ADC_MUX::ADC_MUX(
    std::array<Pot, NUMBER_OF_CONTROLS> &potsRef, CustomADC &customADC)
    : pots(potsRef), adc(customADC)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(channels[i], OUTPUT);
    }

    pinMode(AN_MUX_OUT1, INPUT);
    pinMode(AN_MUX_OUT2, INPUT);
    adc.setInputPin(AN_MUX_OUT1);
    adc.setInputPin(AN_MUX_OUT2);
}

// FixedPoint_EMA(): The returned value and nPrevValue are shifted left by nAlphaShift bits, thus
// retaining the full precision. nSample can be up to (16 - nAlphaShift)-bit wide.
// The real value to use must be shifted right by nAlphaShift bits
// (unless we want to keep the nAlphaShift fractional bits).
//
uint16_t ADC_MUX::FixedPoint_EMA(uint16_t nSample, uint16_t nPrevValue, uint8_t nAlphaShift)
{
    return (uint16_t)(nPrevValue + nSample - (nPrevValue >> nAlphaShift));
}

void ADC_MUX::update(uint16_t sample)
{
	if (curConvChannelIndex >= invalidConvChannelIndex)
	{
		triggerADC(0);
		return;
	}
	
	const uint16_t sensorRead = sample;
	
	Pot &pot = pots[curConvChannelIndex];
	
	uint8_t nextConvChannelIndex = curConvChannelIndex + 1;
	if (nextConvChannelIndex >= NUMBER_OF_CONTROLS)
		nextConvChannelIndex = 0;
	
	triggerADC(nextConvChannelIndex);
	
    constexpr uint8_t nAlphaShift = 3, nDownShift = (12 + nAlphaShift) - 14;
    uint16_t filteredValue_EMA = FixedPoint_EMA(sensorRead, pot.getPreviousValue_EMA(), nAlphaShift);
    // Note: the resolution of filteredValue_EMA is (12 + nAlphaShift) bits.
    // By shifting it right by (12 + nAlphaShift - 14) bits, its resolution becomes 14 bits.
    uint16_t filteredValue = constrain(map(filteredValue_EMA >> nDownShift, 24, 16367, 0, 16383), 0, 16383);

    pot.setCurrentValue(filteredValue);
    pot.setPreviousValue_EMA(filteredValue_EMA);

    uint16_t value_difference = (filteredValue >= pot.getPreviousValue()) ? (filteredValue - pot.getPreviousValue()) : (pot.getPreviousValue() - filteredValue);

    if (pot.getState() == Pot::Pot_t::IDLE)
    {
        if (value_difference > threshold_idle_to_motion)
        {
            pot.setState(Pot::Pot_t::IN_MOTION);
        }
    }
    else if (pot.getState() == Pot::Pot_t::IN_MOTION)
    {
        if (value_difference < threshold_motion_to_idle)
        {
            pot.increaseReleaseCounter();
            if (pot.getReleaseCounter() > 3)
            {
                pot.setState(Pot::Pot_t::IDLE);
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

void ADC_MUX::setMultiplexer(const uint8_t &index)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        digitalWrite(channels[i], bitRead(index, i));
    }
}

void ADC_MUX::triggerADC(const uint8_t &index)
{
	if (index >= invalidConvChannelIndex)
		return;
	
	setMultiplexer(index);
	delayMicroseconds(muxSettlingDelay);
	
	curConvChannelIndex = index;
	adc.trig(pinSelector(index));
}
