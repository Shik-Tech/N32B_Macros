/*
	Hardware Timer (Interrupt Support).

	MIT License

  Copyright (c) 2024 SHIK
  Written by Guillaume Rosanis
*/

#include "HW_Timer.h"

static struct
{
	bool active;
	bool running;
	
	HW_Timer_Callback_t fCallback;
	void * CallbackParam;
}
HW_Timers_Params[HW_Timers_Nb] = {};

#if defined(_SAMD21_)

static const struct
{
	Tc * TCx;
	uint16_t GCLK_ID;
	IRQn_Type IRQn;
}
HW_Timers_SAMD21[HW_Timers_Nb] =
{
	{ TC3, GCLK_CLKCTRL_ID_TCC2_TC3, TC3_IRQn },
	{ TC4, GCLK_CLKCTRL_ID_TC4_TC5, TC4_IRQn },
	{ TC5, GCLK_CLKCTRL_ID_TC4_TC5, TC5_IRQn }
};

static inline void TC_Sync(Tc * TCx) __attribute__((always_inline));
static inline void TC_Sync(Tc * TCx)
{
	while (TCx->COUNT16.STATUS.bit.SYNCBUSY == 1) {};
}

void HW_Timer::init(HW_Timer_Callback_t fCallback, void * CallbackParam)
{
	if ((iTimer < HW_Timers_Nb) && (! HW_Timers_Params[iTimer].active))
	{
		Tc * TCx = HW_Timers_SAMD21[iTimer].TCx;
		uint16_t GCLK_ID = HW_Timers_SAMD21[iTimer].GCLK_ID;
		
		// Set GCLKGEN0 as clock source for TCx.
		GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_ID);
		while (GCLK->STATUS.bit.SYNCBUSY == 1) {}
		
		TCx->COUNT16.CTRLA.bit.ENABLE = 0;
		TC_Sync(TCx);
		
		// Compare mode (periodic).
		TCx->COUNT16.CTRLA.bit.WAVEGEN = TC_CTRLA_WAVEGEN_MFRQ_Val;
		TC_Sync(TCx);
		
		// Enable Compare interrupt.
		TCx->COUNT16.INTENSET.reg = 0;
		TCx->COUNT16.INTENSET.bit.MC0 = 1;
		TC_Sync(TCx);
		
		HW_Timers_Params[iTimer].active = true;
		HW_Timers_Params[iTimer].running = false;
		HW_Timers_Params[iTimer].fCallback = fCallback;
		HW_Timers_Params[iTimer].CallbackParam = CallbackParam;
	}
}

void HW_Timer::setPeriod(uint32_t period_us)
{
	if ((iTimer < HW_Timers_Nb) && HW_Timers_Params[iTimer].active)
	{
		Tc * TCx = HW_Timers_SAMD21[iTimer].TCx;
		
		// Stop timer to configure the new period, if it was running.
		if (HW_Timers_Params[iTimer].running)
		{
			TCx->COUNT16.CTRLA.bit.ENABLE = 0;
			TC_Sync(TCx);
		}
		
		// Determine prescaler and compare value (period).
		constexpr int nPrescalers = 8;
		const uint16_t Prescalers[nPrescalers] = { 1, 2, 4, 8, 16, 64, 256, 1024 };
		constexpr uint32_t nCompMax = 65536;
		
		uint32_t nComp = 0;
		int iPre;
		
		for (iPre = 0; iPre < 8; iPre++)
		{
			nComp = period_us * HW_TIMER_SRC_CLOCK_MHZ;
			nComp += (Prescalers[iPre] / 2);	// Rounding
			nComp /= Prescalers[iPre];
			
			if (nComp <= nCompMax)
				break;
		}
		
		if (nComp == 0)
			nComp = 1;
		
		if (nComp > nCompMax)
			nComp = nCompMax;
		
		if (iPre >= nPrescalers)
			iPre = nPrescalers - 1;
		
		TCx->COUNT16.CTRLA.bit.PRESCALER = iPre & 0x7;
		TC_Sync(TCx);
		
		TCx->COUNT16.CC[0].reg = (uint16_t)(nComp - 1);
		
		TCx->COUNT16.COUNT.reg = 0;
		TC_Sync(TCx);
		
		// Actual period.
		period_us = nComp * Prescalers[iPre];
		period_us += (HW_TIMER_SRC_CLOCK_MHZ / 2);	// Rounding
		period_us /= HW_TIMER_SRC_CLOCK_MHZ;
		
		this->period_us = period_us;
		
		// Re-start timer now, if it was running before calling this method.
		if (HW_Timers_Params[iTimer].running)
		{
			TCx->COUNT16.CTRLA.bit.ENABLE = 1;
			TC_Sync(TCx);
		}
	}
}

void HW_Timer::start()
{
	if ((iTimer < HW_Timers_Nb) && HW_Timers_Params[iTimer].active && (! HW_Timers_Params[iTimer].running))
	{
		Tc * TCx = HW_Timers_SAMD21[iTimer].TCx;
		IRQn_Type TCx_IRQn = HW_Timers_SAMD21[iTimer].IRQn;
		
		if (TCx->COUNT16.INTFLAG.bit.MC0)
			TCx->COUNT16.INTFLAG.bit.MC0 = 1;
		
		NVIC_EnableIRQ(TCx_IRQn);
		
		TCx->COUNT16.COUNT.reg = 0;
		TC_Sync(TCx);
		
		TCx->COUNT16.CTRLA.bit.ENABLE = 1;
		TC_Sync(TCx);
		
		HW_Timers_Params[iTimer].running = true;
	}
}

void HW_Timer::stop()
{
	if ((iTimer < HW_Timers_Nb) && HW_Timers_Params[iTimer].active && HW_Timers_Params[iTimer].running)
	{
		Tc * TCx = HW_Timers_SAMD21[iTimer].TCx;
		IRQn_Type TCx_IRQn = HW_Timers_SAMD21[iTimer].IRQn;
		
		TCx->COUNT16.CTRLA.bit.ENABLE = 0;
		TC_Sync(TCx);
		
		NVIC_DisableIRQ(TCx_IRQn);
		
		HW_Timers_Params[iTimer].running = false;
	}
}

// Interrupt Handlers.
//
#if HW_TIMER_USE_TC3
void TC3_Handler()
{
	if (TC3->COUNT16.INTFLAG.bit.MC0)
	{
		TC3->COUNT16.INTFLAG.bit.MC0 = 1;
		
		if (HW_Timers_Params[0].fCallback != nullptr)
			HW_Timers_Params[0].fCallback(HW_Timers_Params[0].CallbackParam);
	}
}
#endif

#if HW_TIMER_USE_TC4
void TC4_Handler()
{
	if (TC4->COUNT16.INTFLAG.bit.MC0)
	{
		TC4->COUNT16.INTFLAG.bit.MC0 = 1;
		
		if (HW_Timers_Params[1].fCallback != nullptr)
			HW_Timers_Params[1].fCallback(HW_Timers_Params[1].CallbackParam);
	}
}
#endif

#if HW_TIMER_USE_TC5
void TC5_Handler()
{
	if (TC5->COUNT16.INTFLAG.bit.MC0)
	{
		TC5->COUNT16.INTFLAG.bit.MC0 = 1;
		
		if (HW_Timers_Params[2].fCallback != nullptr)
			HW_Timers_Params[2].fCallback(HW_Timers_Params[2].CallbackParam);
	}
}
#endif

#else
#error Target not supported!
#endif

HW_Timer::HW_Timer(uint8_t iTimer)
{
	if (
#if HW_TIMER_USE_TC3
	(iTimer == 0) ||
#endif
#if HW_TIMER_USE_TC4
	(iTimer == 1) ||
#endif
#if HW_TIMER_USE_TC5
	(iTimer == 2) ||
#endif
	false)
	{
		this->iTimer = iTimer;
		
		HW_Timers_Params[iTimer].fCallback = nullptr;
		HW_Timers_Params[iTimer].CallbackParam = nullptr;
	}
	else
	{
		this->iTimer = HW_TIMER_INVALID;
	}
}

void HW_Timer::init(uint32_t period_us, HW_Timer_Callback_t fCallback, void * CallbackParam)
{
	init(fCallback, CallbackParam);
	setPeriod(period_us);
}

void HW_Timer::free()
{
	if (iTimer < HW_Timers_Nb)
	{
		stop();
		
		HW_Timers_Params[iTimer].active = false;
	}
}
