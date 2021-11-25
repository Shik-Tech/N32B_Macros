/*
  N32B Hi Res Firmware v3.0.2
  MIT License

  Copyright (c) 2021 SHIK
*/

#include "mux_factory.h"

MUX_FACTORY::MUX_FACTORY() {}
void MUX_FACTORY::init(uint8_t channel1, uint8_t channel2, uint8_t channel3, int8_t channel4)
{
    currentChannel = 0;

    channels[0] = channel1;
    channels[1] = channel2;
    channels[2] = channel3;
    channels[3] = channel4;
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(channels[i], OUTPUT);
    }
    delay(10);
    // timeout = millis();
}
void MUX_FACTORY::setSignalPin(bool muxIndex, uint8_t pin)
{
    signalPin[muxIndex] = pin;
    pinMode(pin, INPUT);
}

void MUX_FACTORY::update(uint8_t currentKnob)
{
    // if (millis() - timeout >= 1)
    // {
    //     read();
    //     timeout = millis();
    // }

    // if (millis() - timeout >= 1)
    // {
        setMultiplexer(currentKnob);
        readSingle(currentKnob);
        // timeout = millis();
    // }
}

// void MUX_FACTORY::read()
// {
//     for (uint8_t i = 0; i < 2; i++)
//     {
//         uint8_t index = (i * 16) + currentChannel;
//         if (index < NUMBER_OF_KNOBS)
//         {
//             for (uint8_t j = 3; j > 0; j--)
//             {
//                 knobBuffer[j][index] = knobBuffer[j - 1][index];
//             }
//             if (activePreset.highResolution)
//             {
//                 knobBuffer[0][index] = analogRead(signalPin[i]);
//             }
//             else
//             {
//                 knobBuffer[0][index] = analogRead(signalPin[i]) >> 3;
//             }
//         }
//     }
//     currentChannel++;
//     if (currentChannel >= NUMBER_OF_KNOBS / 2)
//     {
//         currentChannel = 0;
//     }
//     for (uint8_t i = 0; i < 4; i++)
//     {
//         digitalWrite(channels[i], bitRead(currentChannel, i));
//     }
// }

// void MUX_FACTORY::readSingle(uint8_t currentKnob)
// {
//     uint8_t bitReducer = activePreset.highResolution ? 0 : 3;
//     bool pinSelector = currentKnob > 15 ? 1 : 0;

//     for (uint8_t j = NUMBER_OF_BUFFERS - 1; j > 0; j--)
//     {
//         knobBuffer[j][currentKnob] = knobBuffer[j - 1][currentKnob];
//     }
//     knobBuffer[0][currentKnob] = analogRead(signalPin[pinSelector]) >> bitReducer;
//     averageValues[currentKnob] = getAverage(currentKnob);
// }

uint16_t MUX_FACTORY::readSingle(uint8_t currentKnob)
{
    uint8_t bitReducer = activePreset.highResolution ? 0 : 3;
    bool pinSelector = currentKnob > 15 ? 1 : 0;

    return analogRead(signalPin[pinSelector]) >> bitReducer;

    // for (uint8_t j = NUMBER_OF_BUFFERS - 1; j > 0; j--)
    // {
    //     knobBuffer[j][currentKnob] = knobBuffer[j - 1][currentKnob];
    // }
    // knobBuffer[0][currentKnob] = analogRead(signalPin[pinSelector]) >> bitReducer;
    // averageValues[currentKnob] = getAverage(currentKnob);
}

void MUX_FACTORY::setMultiplexer(uint8_t currentKnob)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        digitalWrite(channels[i], bitRead(currentKnob, i));
    }
}

// uint16_t MUX_FACTORY::getAverage(uint8_t index)
// {
//     uint16_t average = 0;
//     for (uint8_t i = 0; i < NUMBER_OF_BUFFERS; i++)
//     {
//         average += knobBuffer[i][index];
//     }
//     average /= NUMBER_OF_BUFFERS;

// #ifdef MK2
//     return 1023 - average;
// #endif

// #ifndef MK2
//     return average;
// #endif
// }