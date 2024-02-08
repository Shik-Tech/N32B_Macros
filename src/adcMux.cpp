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

uint8_t counter = 0;
void ADC_MUX::update(const uint8_t &index)
{
    setMultiplexer(index);
    delayMicroseconds(10);
    const uint16_t sensorRead = read(index);

    Pot &pot = pots[index];
    uint16_t filteredValue = FixedPoint_EMA(sensorRead, pot.getPreviousValue(), 4);
    // const uint16_t currentThresholdValue = filteredValue >> 2;
    // const uint16_t previousThresholdValue = ;

    // if (index == 0)
    // {
    //     if (counter == 0 || counter > 499)
    //     {
    //         Serial.println("---------");
    //         delay(5000);
    //         counter = 0;
    //     }
    //     if (counter < 500)
    //     {
    //         Serial.println(sensorRead);
    //         counter++;
    //     }
    // }
    if (index == 0)
    {
        Serial.print("sensorRead: ");
        Serial.println(sensorRead);
        Serial.print("filteredValue: ");
        Serial.println(filteredValue);
    }

    // filteredValue = constrain(map(filteredValue, 15, 16368, 0, 16383), 0, 16383);
    pot.setCurrentValue(filteredValue);

    // uint16_t value_difference = abs(static_cast<int>(pot.getCurrentValue()) - static_cast<int>(pot.getPreviousValue()));
    // uint16_t value_difference = abs((int) pot.getCurrentValue() - (int) pot.getPreviousValue());
    uint16_t value_difference = (filteredValue >= pot.getPreviousValue()) ? (filteredValue - pot.getPreviousValue()) : (pot.getPreviousValue() - filteredValue);

    // uint16_t value_difference = (currentThresholdValue >= pot.getPreviousThresholdValue()) ? (currentThresholdValue - pot.getPreviousThresholdValue()) : (pot.getPreviousThresholdValue() - currentThresholdValue);

    // if (index == 0)
    // {
    //     Serial.println(sensorRead);
    //     Serial.println(filteredValue);
    //     Serial.println("---------");
    // }

    if (pot.getState() == Pot_t::IDLE)
    {
        // if (value_difference > threshold_idle_to_motion && index != 0)
        // {
        //     // Print the pot's index, sensorRead, filteredValue, and value_difference to Serial
        //     Serial.print("Pot Index: ");
        //     Serial.print(index);
        //     Serial.print(", Sensor Read: ");
        //     Serial.print(sensorRead);
        //     Serial.print(", Filtered Value: ");
        //     Serial.print(filteredValue);
        //     Serial.print(", Current Threshold Value Shifted: ");
        //     Serial.print(currentThresholdValue);
        //     Serial.print(", Value Difference: ");
        //     Serial.println(value_difference);
        // }
        if (value_difference > threshold_idle_to_motion)
        {
            // pot.increaseMotionCounter();
            // if (pot.getMotionCounter() > 3)
            // {
            pot.setState(Pot_t::IN_MOTION);
            //     pot.resetMotionCounter();
            // }
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
    // pot.setPreviousValue();
    // pot.setPreviousThresholdValue(currentThresholdValue);
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