/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "CustomADC.h"

inline uint8_t CustomADC::getAvgCtrlReg(uint16_t nSamples)
{
	if (nSamples > 768)
		return ADC_AVGCTRL_SAMPLENUM_1024 | ADC_AVGCTRL_ADJRES(0x4);
	if (nSamples > 384)
		return ADC_AVGCTRL_SAMPLENUM_512 | ADC_AVGCTRL_ADJRES(0x4);
	if (nSamples > 192)
		return ADC_AVGCTRL_SAMPLENUM_256 | ADC_AVGCTRL_ADJRES(0x4);
	if (nSamples > 96)
		return ADC_AVGCTRL_SAMPLENUM_128 | ADC_AVGCTRL_ADJRES(0x4);
	if (nSamples > 48)
		return ADC_AVGCTRL_SAMPLENUM_64 | ADC_AVGCTRL_ADJRES(0x4);
	if (nSamples > 24)
		return ADC_AVGCTRL_SAMPLENUM_32 | ADC_AVGCTRL_ADJRES(0x4);
	if (nSamples > 12)
		return ADC_AVGCTRL_SAMPLENUM_16 | ADC_AVGCTRL_ADJRES(0x4);
	if (nSamples > 6)
		return ADC_AVGCTRL_SAMPLENUM_8 | ADC_AVGCTRL_ADJRES(0x3);
	if (nSamples > 3)
		return ADC_AVGCTRL_SAMPLENUM_4 | ADC_AVGCTRL_ADJRES(0x2);
	if (nSamples > 1)
		return ADC_AVGCTRL_SAMPLENUM_2 | ADC_AVGCTRL_ADJRES(0x1);
	
	return ADC_AVGCTRL_SAMPLENUM_1 | ADC_AVGCTRL_ADJRES(0x0);
}

#ifdef _SAMD21_

// Interrupt Callback.
//
static CustomADC_Callback_t CustomADC_Callback = nullptr;
static void * CustomADC_Callback_Param = nullptr;

// Wait for synchronization of registers between the clock domains.
//
#define ADC_SYNC()			while (ADC->STATUS.bit.SYNCBUSY == 1) {}

// Wait for the current conversion to complete.
//
static inline void waitConvADC(void)  __attribute__((always_inline));
static inline void waitConvADC(void)
{
	while (ADC->INTFLAG.bit.RESRDY == 0) {}
	
	ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
	ADC_SYNC();
}

// Initialization of the ADC for the SBrain / N32B Board SAMD21).
//
void CustomADC::init(
	uint8_t nSamplingCycles, uint16_t nAvgSamples,
	CustomADC_Callback_t fCallback, void * CallbackParam)
{
	// Disable ADC to configure it.
	ADC->CTRLA.bit.ENABLE = 0;
	ADC_SYNC();
	
	// Adjust ADC clock to 1.5MHz @48MHz system clock (max ADC clock = 2.1MHz)
	// The 16-bit mode is required for the averaging mode (cf. datasheet).
	ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV32 | ADC_CTRLB_RESSEL_16BIT;
	ADC_SYNC();
	
	// Sampling time.
	if (nSamplingCycles > 63)
		nSamplingCycles = 63;
	
	ADC->SAMPCTRL.reg = nSamplingCycles;
	
	// Average mode control. The final value in the RESULT register has a 12-bit resolution.
	// (Cf. datasheet, in particular 33.6.6 & 33.6.7)
	ADC->AVGCTRL.reg = getAvgCtrlReg(nAvgSamples);
	ADC_SYNC();
	
	// Set gain and analog reference.
	ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
	ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val;
	ADC_SYNC();
	
	// Interrupt.
	CustomADC_Callback = fCallback;
	CustomADC_Callback_Param = CallbackParam;
	
	if (fCallback != nullptr)
	{
		// Enable Result Ready interrupt.
		ADC->INTENSET.reg = ADC_INTENSET_RESRDY;
		NVIC_EnableIRQ(ADC_IRQn);
	}
	
	// Enable ADC which remains permanently enabled.
	ADC->CTRLA.bit.ENABLE = 1;
	ADC_SYNC();
}

// Select the analog input and trigger a new conversion, return immediately after that.
//
void CustomADC::trig(uint8_t pin)
{
	ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[pin].ulADCChannelNumber;
	ADC_SYNC();
	
	ADC->SWTRIG.bit.START = 1;
	ADC_SYNC();
}

// Wait for a conversion that has been triggered to complete and return the result.
//
uint16_t CustomADC::getConv(void)
{
	waitConvADC();
	return (uint16_t) ADC->RESULT.reg;
}
// Select the analog input, trigger a new conversion, wait for it to complete and return the result.
//
uint16_t CustomADC::read(uint8_t pin)
{
	trig(pin);
	return getConv();
}

#elif defined(_SAMD51_)

// Interrupt Callback.
//
static CustomADC_Callback_t CustomADC_Callback = nullptr;
static void * CustomADC_Callback_Param = nullptr;

// Wait for synchronization of registers between the clock domains.
// On the SAMD51, there's a sync flag per ADC register.
//
#define ADC0_SYNC(Reg)		while (ADC0->SYNCBUSY.bit.Reg == 1) {}

// Wait for the current conversion to complete.
//
static inline void waitConvADC0() __attribute__((always_inline));
static inline void waitConvADC0()
{
	while (ADC0->INTFLAG.bit.RESRDY == 0) {}
	
	ADC0->INTFLAG.reg = ADC_INTFLAG_RESRDY;
}

// Initialization of the ADC0 for the SBrain Board SAMD51).
//
void CustomADC::init(
	uint8_t nSamplingCycles, uint16_t nAvgSamples,
	CustomADC_Callback_t fCallback, void * CallbackParam)
{
	// Disable ADC to configure it.
	ADC0->CTRLA.bit.ENABLE = 0;
	ADC0_SYNC(ENABLE);
	
	// Adjust ADC clock to 6MHz @48MHz GCLK1.
	// (The Arduino core sets GCLK_ADCx to GCLK1, which itself is set @48MHz. See 'wiring.c')
	ADC0->CTRLA.reg = ADC_CTRLA_PRESCALER_DIV8;
	
	// The 16-bit mode is required for the averaging mode (cf. datasheet).
	ADC0->CTRLB.reg = ADC_CTRLB_RESSEL_16BIT;
	ADC0_SYNC(CTRLB);
	
	// Sampling time.
	if (nSamplingCycles > 0)
		nSamplingCycles -= 1;
	
	if (nSamplingCycles > 63)
		nSamplingCycles = 63;
	
	ADC0->SAMPCTRL.reg = nSamplingCycles;
	ADC0_SYNC(SAMPCTRL);
	
	// Average mode control. The final value in the RESULT register has a 12-bit resolution.
	// (Cf. datasheet, in particular 45.6.2.10)
	ADC0->AVGCTRL.reg = getAvgCtrlReg(nAvgSamples);//ADC_AVGCTRL_SAMPLENUM_128 | ADC_AVGCTRL_ADJRES(0x04);
	ADC0_SYNC(AVGCTRL);
	
	// Set gain and analog reference.
	/*ADC0->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
	ADC0_SYNC(INPUTCTRL);*/
	ADC0->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_INTVCC1_Val;
	ADC0_SYNC(REFCTRL);
	
	// Interrupt.
	CustomADC_Callback = fCallback;
	CustomADC_Callback_Param = CallbackParam;
	
	if (fCallback != nullptr)
	{
		// Enable Result Ready interrupt.
		ADC0->INTENSET.reg = ADC_INTENSET_RESRDY;
		NVIC_EnableIRQ(ADC0_1_IRQn);
	}
	
	// Enable ADC which remains permanently enabled.
	ADC0->CTRLA.bit.ENABLE = 1;
	ADC0_SYNC(ENABLE);
}

// Select the analog input and trigger a new conversion, return immediately after that.
//
void CustomADC::trig(uint8_t pin)
{
	ADC0->INPUTCTRL.bit.MUXPOS = g_APinDescription[pin].ulADCChannelNumber;
	ADC0_SYNC(INPUTCTRL);
	
	ADC0->SWTRIG.bit.START = 1;
	ADC0_SYNC(SWTRIG);
}

// Wait for a conversion that has been triggered to complete and return the result.
//
uint16_t CustomADC::getConv(void)
{
	waitConvADC0();
	return (uint16_t) ADC0->RESULT.reg;
}
// Select the analog input, trigger a new conversion, wait for it to complete and return the result.
//
uint16_t CustomADC::read(uint8_t pin)
{
	trig(pin);
	return getConv();
}

#else
#error Target not supported!
#endif

// Configure a given pin as analog input.
//
void CustomADC::setInputPin(uint32_t ulPin)
{
	// Handle the case the pin isn't usable as PIO
	if (g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN)
	{
		return;
	}
	
	if (g_APinDescription[ulPin].ulPin & 1) // is pin odd?
	{
		uint32_t temp;

		// Get whole current setup for both odd and even pins and remove odd one
		temp = (PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg) & PORT_PMUX_PMUXE( 0xF ) ;
		// Set new muxing
		PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg = temp|PORT_PMUX_PMUXO(PIO_ANALOG) ;
		// Enable port mux
		PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg |= PORT_PINCFG_PMUXEN ;
	}
	else // even pin
	{
		uint32_t temp;

		temp = (PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg) & PORT_PMUX_PMUXO( 0xF ) ;
		PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg = temp|PORT_PMUX_PMUXE(PIO_ANALOG) ;
		PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg |= PORT_PINCFG_PMUXEN ; // Enable port mux
	}
}

CustomADC::CustomADC()
{
	CustomADC_Callback = nullptr;
	CustomADC_Callback_Param = nullptr;
}

// Interrupt Handler.
//
#ifdef _SAMD21_
void ADC_Handler()
{
	if (ADC->INTFLAG.bit.RESRDY)
	{
		ADC->INTFLAG.reg = ADC_INTFLAG_RESRDY;
		
		if (CustomADC_Callback != nullptr)
		{
			uint16_t sample = (uint16_t) ADC->RESULT.reg;
			
			CustomADC_Callback(sample, CustomADC_Callback_Param);
		}
	}
}
#elif defined(_SAMD51_)
void ADC0_1_Handler()
{
	if (ADC0->INTFLAG.bit.RESRDY)
	{
		ADC0->INTFLAG.reg = ADC_INTFLAG_RESRDY;
		
		if (CustomADC_Callback != nullptr)
		{
			uint16_t sample = (uint16_t) ADC0->RESULT.reg;
			
			CustomADC_Callback(sample, CustomADC_Callback_Param);
		}
	}
}
#endif
