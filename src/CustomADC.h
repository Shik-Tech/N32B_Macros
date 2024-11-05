/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#ifndef CUSTOMADC_h
#define CUSTOMADC_h

//#include <Arduino.h>
#include "variant.h"

typedef void (*CustomADC_Callback_t)(uint16_t sample, void * param);

class CustomADC
{
	private:
		inline uint8_t getAvgCtrlReg(uint16_t nSamples);
	
	public:
		CustomADC();
		
		void init(
			uint8_t nSamplingCycles, uint16_t nAvgSamples,
			CustomADC_Callback_t fCallback, void * CallbackParam);
		
		void setInputPin(uint32_t ulPin);
		
		void trig(uint8_t pin);
		uint16_t getConv(void);
		uint16_t read(uint8_t pin);
};

#endif
