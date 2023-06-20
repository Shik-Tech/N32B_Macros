/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2023 SHIK
*/

#include "mux_factory.h"

MUX_FACTORY::MUX_FACTORY() {}
void MUX_FACTORY::init(const uint8_t &channel1, const uint8_t &channel2, const uint8_t &channel3, const uint8_t &channel4)
{
    channels[0] = channel1;
    channels[1] = channel2;
    channels[2] = channel3;
    channels[3] = channel4;
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(channels[i], OUTPUT);
    }
    delay(10);
}
void MUX_FACTORY::setSignalPin(const bool &muxIndex, const uint8_t &pin)
{
    signalPin[muxIndex] = pin;
    pinMode(pin, INPUT);
}

void MUX_FACTORY::update(uint8_t &index)
{
    setMultiplexer(index);
    uint16_t currentValue = read(index);
    int diff = device.knobValues[index][0] != device.knobValues[index][1];
    int value = (EMA_a)*currentValue + (1 - EMA_a) * device.knobValues[index][1];
    device.knobValues[index][0] = constrain(map(value, 0, 1020, 0, 1023), 0, 1023);
    if (diff)
    {
        device.knobValues[index][1] = device.knobValues[index][0];
    }
}

uint16_t MUX_FACTORY::read(uint8_t &index)
{
    bool pinSelector = index > 15 ? 1 : 0;
    return analogRead(signalPin[pinSelector]);
}

void MUX_FACTORY::setMultiplexer(uint8_t &index)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        digitalWrite(channels[i], bitRead(index, i));
    }
}