/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2023 SHIK
*/

#include "adcMux.h"

ADC_MUX::ADC_MUX(
    uint8_t MUX_A_SIG,
    uint8_t MUX_B_SIG,
    uint8_t MUX_S0,
    uint8_t MUX_S1,
    uint8_t MUX_S2,
    uint8_t MUX_S3,
    Pot *potsPtr)
    : mux_a_sig(MUX_A_SIG),
      mux_b_sig(MUX_B_SIG),
      muxS0(MUX_S0),
      muxS1(MUX_S1),
      muxS2(MUX_S2),
      muxS3(MUX_S3),
      pots(potsPtr) {}

void ADC_MUX::init()
{
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(channels[i], OUTPUT);
    }

    pinMode(mux_a_sig, INPUT);
    pinMode(mux_b_sig, INPUT);
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

    int value_difference = abs(pot.current_value - pot.previous_value);
    if (index == 0)
    {
        Serial.println(pot.current_value);
    }
    if (pot.state == Pot::IDLE)
    {
        if (value_difference > threshold_idle_to_motion)
        {
            pot.state = Pot::IN_MOTION;

            // if (index == 0)
            // {
            //     SerialUSB.print("previous_value: ");
            //     SerialUSB.println(pot.previous_value);

            //     SerialUSB.print("sensorRead: ");
            //     SerialUSB.println(sensorRead);

            //     SerialUSB.print("current_value: ");
            //     SerialUSB.println(pot.current_value);

            //     SerialUSB.print("value_difference: ");
            //     SerialUSB.println(value_difference);

            //     SerialUSB.print("release_counter: ");
            //     SerialUSB.println(pot.release_counter);

            //     SerialUSB.println("SET IN_MOTION");
            //     SerialUSB.println("------------");
            // }
            pot.previous_value = pot.current_value;
        }
    }
    else if (pot.state == Pot::IN_MOTION)
    {
        // if (index == 0)
        // {
        //     SerialUSB.println("IN_MOTION");

        //     SerialUSB.print("previous_value: ");
        //     SerialUSB.println(pot.previous_value);

        //     SerialUSB.print("sensorRead: ");
        //     SerialUSB.println(sensorRead);

        //     SerialUSB.print("current_value: ");
        //     SerialUSB.println(pot.current_value);

        //     SerialUSB.print("value_difference: ");
        //     SerialUSB.println(value_difference);

        //     SerialUSB.print("release_counter: ");
        //     SerialUSB.println(pot.release_counter);

        //     SerialUSB.println("------------");
        // }

        if (value_difference < threshold_motion_to_idle)
        {
            pot.release_counter++;
            if (pot.release_counter > 3)
            {
                pot.state = Pot::IDLE;

                if (index == 0)
                {
                    SerialUSB.print("previous_value: ");
                    SerialUSB.println(pot.previous_value);

                    SerialUSB.print("sensorRead: ");
                    SerialUSB.println(sensorRead);

                    SerialUSB.print("current_value: ");
                    SerialUSB.println(pot.current_value);

                    SerialUSB.print("value_difference: ");
                    SerialUSB.println(value_difference);

                    SerialUSB.print("release_counter: ");
                    SerialUSB.println(pot.release_counter);

                    SerialUSB.println("IDLE");
                    SerialUSB.println("------------");
                }
                pot.release_counter = 0;
            }
        }
        else
        {
            pot.release_counter = 0;
            pot.previous_value = pot.current_value;
        }
    }
    // pot.previous_value = pot.current_value;
}

uint8_t ADC_MUX::pinSelector(const uint8_t &index)
{
    return signalPin[index > 15 ? 1 : 0];
}

uint16_t ADC_MUX::read(const uint8_t &index)
{
    // return SBrain_ADC_Read(pinSelector(index));
    return analogRead(pinSelector(index));
}

void ADC_MUX::setMultiplexer(const uint8_t &index)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        digitalWrite(channels[i], bitRead(index, i));
    }
}

// void ADC_MUX::init(const uint8_t &channel1, const uint8_t &channel2, const uint8_t &channel3, const uint8_t &channel4)
// {
//     channels[0] = channel1;
//     channels[1] = channel2;
//     channels[2] = channel3;
//     channels[3] = channel4;
//     for (uint8_t i = 0; i < 4; i++)
//     {
//         pinMode(channels[i], OUTPUT);
//     }
//     delay(10);
// }
// void ADC_MUX::setSignalPin(const bool &muxIndex, const uint8_t &pin)
// {
//     signalPin[muxIndex] = pin;
//     pinMode(pin, INPUT);
// }

// void ADC_MUX::update(uint8_t &index)
// {
//     setMultiplexer(index);
//     uint16_t currentValue = read(index);
//     int diff = device.knobValues[index][0] != device.knobValues[index][1];
//     int value = (EMA_a)*currentValue + (1 - EMA_a) * device.knobValues[index][1];
//     device.knobValues[index][0] = constrain(map(value, 0, 1020, 0, 1023), 0, 1023);
//     if (diff)
//     {
//         device.knobValues[index][1] = device.knobValues[index][0];
//     }
// }

// uint16_t ADC_MUX::read(uint8_t &index)
// {
//     bool pinSelector = index > 15 ? 1 : 0;
//     return analogRead(signalPin[pinSelector]);
// }

// void ADC_MUX::setMultiplexer(uint8_t &index)
// {
//     for (uint8_t i = 0; i < 4; i++)
//     {
//         digitalWrite(channels[i], bitRead(index, i));
//     }
// }