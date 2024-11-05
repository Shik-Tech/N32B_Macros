/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "Display.h"

// Auto clear the display
void Display::clearDisplay(uint16_t readInterval)
{
    if (millis() - displayOffTimer >= readInterval)
    {
        clear();
        flush();
    }
}

void Display::showValue(uint8_t value, uint8_t index)
{
    if (activeKnobIndex == -1)
    {
        setActiveKnobIndex(index);
    }

    if (activeKnobIndex != index)
    {
        return;
    }

    setNumber(value);
    flush();

    // lastUpdateTime = millis();
    displayOffTimer = millis();
}

// Blink the decimal points
void Display::blinkDot(uint8_t nDigit)
{
    setDecimalPoint(nDigit);
    flush();

    displayOffTimer = millis();
}

void Display::showChannelNumber(uint8_t channelNumber)
{
    setNumber(channelNumber);
    setDigit(2, MAX7219DIGIT(0b01001110));
    flush();

    displayOffTimer = millis();
}

void Display::showPresetNumber(uint8_t presetNumber)
{
    setNumber(presetNumber + 1);
    setDigit(2, MAX7219DIGIT(0b01100111));
    flush();

    displayOffTimer = millis();
}

void Display::showStartUpAnimation()
{
    uint8_t delayTime = 120;
    clear();

    setDigit(2, MAX7219DIGIT(0b00001000));
    setDigit(1, MAX7219DIGIT(0b00000000));
    setDigit(0, MAX7219DIGIT(0b00000000));
    flush();

    delay(delayTime);

    setDigit(2, MAX7219DIGIT(0b00001000));
    setDigit(1, MAX7219DIGIT(0b00001000));
    setDigit(0, MAX7219DIGIT(0b00000000));
    flush();

    delay(delayTime);

    setDigit(2, MAX7219DIGIT(0b00001000));
    setDigit(1, MAX7219DIGIT(0b00001000));
    setDigit(0, MAX7219DIGIT(0b00001000));
    flush();

    delay(delayTime);

    setDigit(2, MAX7219DIGIT(0b00001000));
    setDigit(1, MAX7219DIGIT(0b00001000));
    setDigit(0, MAX7219DIGIT(0b00011000));
    flush();

    delay(delayTime);

    setDigit(2, MAX7219DIGIT(0b00001000));
    setDigit(1, MAX7219DIGIT(0b00001000));
    setDigit(0, MAX7219DIGIT(0b00111000));
    flush();

    delay(delayTime);

    setDigit(2, MAX7219DIGIT(0b00001000));
    setDigit(1, MAX7219DIGIT(0b00001000));
    setDigit(0, MAX7219DIGIT(0b01111000));
    flush();

    delay(delayTime);

    setDigit(2, MAX7219DIGIT(0b00001000));
    setDigit(1, MAX7219DIGIT(0b01001000));
    setDigit(0, MAX7219DIGIT(0b01111000));
    flush();

    delay(delayTime);

    setDigit(2, MAX7219DIGIT(0b01001000));
    setDigit(1, MAX7219DIGIT(0b01001000));
    setDigit(0, MAX7219DIGIT(0b01111000));
    flush();

    delay(delayTime);

    setDigit(2, MAX7219DIGIT(0b01001010));
    setDigit(1, MAX7219DIGIT(0b01001000));
    setDigit(0, MAX7219DIGIT(0b01111000));
    flush();

    delay(delayTime);

    setDigit(2, MAX7219DIGIT(0b01001110));
    setDigit(1, MAX7219DIGIT(0b01001000));
    setDigit(0, MAX7219DIGIT(0b01111000));
    flush();

    delay(300);

    clear();
    flush();

    displayOffTimer = millis();
}

// Show animation after factory reset (infinity symbol animation)
void Display::factoryResetAnimation()
{
    const uint8_t delayTime = 100;
    const uint8_t repeats = 3;

    for (uint8_t i = 0; i < repeats; i++)
    {
        clear();

        setDigit(1, MAX7219DIGIT(0b00010000));
        flush();
        delay(delayTime);

        setDigit(1, MAX7219DIGIT(0b00011000));
        flush();
        delay(delayTime);

        setDigit(1, MAX7219DIGIT(0b00011100));
        flush();
        delay(delayTime);

        setDigit(1, MAX7219DIGIT(0b00001101));
        flush();
        delay(delayTime);

        setDigit(1, MAX7219DIGIT(0b00000101));
        setDigit(0, MAX7219DIGIT(0b00000001));
        flush();
        delay(delayTime);

        setDigit(1, MAX7219DIGIT(0b00000001));
        setDigit(0, MAX7219DIGIT(0b00100001));
        flush();
        delay(delayTime);

        setDigit(0, MAX7219DIGIT(0b01100001));
        flush();
        delay(delayTime);

        setDigit(0, MAX7219DIGIT(0b01100010));
        flush();
        delay(delayTime);

        setDigit(0, MAX7219DIGIT(0b01000010));
        setDigit(1, MAX7219DIGIT(0b00010000));
        flush();
        delay(delayTime);

        clear();
        flush();
    }
}

// Show save message "Sv."
void Display::showSaveMessage()
{
    const uint16_t delayTime = 300;

    clear();

    setDigit(1, MAX7219DIGIT(0b01011011));
    setDigit(0, MAX7219DIGIT(0b00011100));
    flush();
    delay(delayTime);

    const uint8_t repeats = 3;
    bool bDecimalPoint = true;

    for (uint8_t i = 0; i < repeats; i++)
    {
        setDecimalPoint(0, bDecimalPoint);
        flush();
        delay(delayTime);

        bDecimalPoint = !bDecimalPoint;
    }
}

void Display::showSynching()
{
    clear();

    setDigit(2, MAX7219DIGIT(0b01011011));
    setDigit(1, MAX7219DIGIT(0b00010101));
    setDigit(0, MAX7219DIGIT(0b00001101));
    flush();
    delay(1000);
}

int8_t Display::getActiveKnobIndex()
{
    return activeKnobIndex;
}
void Display::setActiveKnobIndex(uint8_t index)
{
    activeKnobIndex = index;
}
void Display::resetActiveKnobIndex()
{
    activeKnobIndex = -1;
}
