/*
  7-Segment LED Display Driver for the TLC59282
  MIT License

  Copyright (c) 2024 Guillaume Rosanis
*/

#ifndef TLC59282_DISPLAY_h
#define TLC59282_DISPLAY_h

#include <Arduino.h>

#define MAX7219DIGIT(data) \
	(\
	 (((data) & 0x80)? 0x80 : 0) | (((data) & 0x40)? 0x01 : 0) | (((data) & 0x20)? 0x02 : 0) | \
	 (((data) & 0x10)? 0x04 : 0) | (((data) & 0x08)? 0x08 : 0) | (((data) & 0x04)? 0x10 : 0) | \
	 (((data) & 0x02)? 0x20 : 0) | (((data) & 0x01)? 0x40 : 0) \
	)

constexpr uint8_t Display_NbDigits = 3;

class TLC59282_Display
{
	private:
		uint8_t Pin_SIN;
		uint8_t Pin_SCLK;
		uint8_t Pin_LAT;
		uint8_t Pin_BLANK;
		
		uint8_t Digits[Display_NbDigits];
		
		void toggleLAT();
		void writeByte(uint8_t data);
	
	public:
		TLC59282_Display(uint8_t SIN, uint8_t SCLK, uint8_t LAT, uint8_t BLANK);
		
		void on();
		void off();
		void flush(uint8_t nbDigits = Display_NbDigits);
		
		void clear();
		
		// nDigit in 0..2.
		void setDigit(uint8_t nDigit, uint8_t data);
		void setNumber(uint16_t number, bool bLeadingZeroes = false);
		void setDecimalPoint(uint8_t nDigit, bool bOn = true);
		
		void shiftLeft(uint8_t nbDigits = 1);
		void shiftRight(uint8_t nbDigits = 1);
};

#endif

