/*
  7-Segment LED Display Driver for the TLC59282
  MIT License

  Copyright (c) 2025 SHIK
  Written by Guillaume Rosanis
*/

#include "TLC59282_Display.h"

// LED segment values for digits 0..9.
static const uint8_t Digits7S[] PROGMEM = {
	B00111111, B00000110, B01011011, B01001111, B01100110,
	B01101101, B01111101, B00000111, B01111111, B01101111
};

static constexpr uint8_t DecimalPoint7S = B10000000;

#ifdef __AVR__
#define TLC59282_GET7SDIGIT(n)		pgm_read_byte_near(Digits7S + (n))
#else
#define TLC59282_GET7SDIGIT(n)		Digits7S[n]
#endif

TLC59282_Display::TLC59282_Display(uint8_t SIN, uint8_t SCLK, uint8_t LAT, uint8_t BLANK)
{
	Pin_SIN = SIN;
	Pin_SCLK = SCLK;
	Pin_LAT = LAT;
	Pin_BLANK = BLANK;
	
	pinMode(Pin_SIN, OUTPUT);
	pinMode(Pin_SCLK, OUTPUT);
	pinMode(Pin_LAT, OUTPUT);
	pinMode(Pin_BLANK, OUTPUT);
	
	digitalWrite(Pin_SIN, LOW);
	digitalWrite(Pin_SCLK, LOW);
	digitalWrite(Pin_LAT, LOW);
	digitalWrite(Pin_BLANK, HIGH);
	
	clear();
	bChanged = false;
	bOn = false;
}

void TLC59282_Display::on()
{
	if (!bOn)
	{
		digitalWrite(Pin_BLANK, LOW);
		bOn = true;
		bChanged = true;
	}
}

void TLC59282_Display::off()
{
	if (bOn)
	{
		digitalWrite(Pin_BLANK, HIGH);
		bOn = false;
		bChanged = true;
	}
}

void TLC59282_Display::toggleLAT()
{
	digitalWrite(Pin_LAT, HIGH);
	digitalWrite(Pin_LAT, LOW);
}

void TLC59282_Display::writeByte(uint8_t data)
{
	// Write data to the serial input, MSB first.
	for (uint8_t i = 0; i < 8; i++, data <<= 1)
	{
		digitalWrite(Pin_SIN, (data & 0x80)? HIGH : LOW);
		
		digitalWrite(Pin_SCLK, HIGH);
		digitalWrite(Pin_SCLK, LOW);
	}
}

void TLC59282_Display::flush(uint8_t nbDigits)
{
	if (nbDigits > Display_NbDigits)
		nbDigits = Display_NbDigits;
	
	toggleLAT();
	
	for (uint8_t i = nbDigits; i > 0; i--)
		writeByte(Digits[i - 1]);
	
	toggleLAT();
	
	if (bOn)
		bChanged = true;
}

void TLC59282_Display::clear()
{
	for (uint8_t i = 0; i < Display_NbDigits; i++)
		Digits[i] = 0x00;
}

void TLC59282_Display::setDigit(uint8_t nDigit, uint8_t data)
{
	if (nDigit < Display_NbDigits)
	{
		Digits[nDigit] = data;
	}
}

void TLC59282_Display::setDecimalPoint(uint8_t nDigit, bool bOn)
{
	if (nDigit < Display_NbDigits)
	{
		if (bOn)
			Digits[nDigit] |= DecimalPoint7S;
		else
			Digits[nDigit] &= (uint8_t)(~DecimalPoint7S);
	}
}

void TLC59282_Display::setNumber(uint16_t number, bool bLeadingZeroes)
{
	if (number > 999)
		number = 999;
	
	// This method of extracting BCD from an integer is much more efficient
	// than division and modulo on a 8-bit MCU without hardware divider.
	uint8_t nP100;
	
	for (nP100 = 0; number >= 100; number -= 100)
		nP100++;
	
	// Don't show the leading zero.
	if ((! bLeadingZeroes) && (nP100 == 0))
		setDigit(2, 0x00);
	else
		setDigit(2, TLC59282_GET7SDIGIT(nP100));
	
	uint8_t nP10;
	
	for (nP10 = 0; number >= 10; number -= 10)
		nP10++;
	
	// Don't show the leading zero.
	if ((! bLeadingZeroes) && ((nP100 == 0) && (nP10 == 0)))
		setDigit(1, 0x00);
	else
		setDigit(1, TLC59282_GET7SDIGIT(nP10));
	
	setDigit(0, TLC59282_GET7SDIGIT(number));
}

void TLC59282_Display::shiftLeft(uint8_t nbDigits)
{
	if (nbDigits > Display_NbDigits)
		nbDigits = Display_NbDigits;
	
	uint8_t i;
	
	for (i = Display_NbDigits; (i > 0) && (i > nbDigits); i--)
		Digits[i - 1] = Digits[i - 1 - nbDigits];
	
	for (; i > 0; i--)
		Digits[i - 1] = 0x00;
}

void TLC59282_Display::shiftRight(uint8_t nbDigits)
{
	if (nbDigits > Display_NbDigits)
		nbDigits = Display_NbDigits;
	
	uint8_t i;
	
	for (i = 0; (i + nbDigits) < Display_NbDigits; i++)
		Digits[i] = Digits[i + nbDigits];
	
	for (; i < Display_NbDigits; i++)
		Digits[i] = 0x00;
}
