/*
	Hardware Timer (Interrupt Support).

	MIT License

  Copyright (c) 2024 SHIK
  Written by Guillaume Rosanis
*/

#ifndef HW_TIMER_h
#define HW_TIMER_h

#include <Arduino.h>

#if defined(_SAMD21_)

constexpr uint8_t HW_Timers_Nb = 3;

#define HW_TIMER_USE_TC3	1
#define HW_TIMER_USE_TC4	1
#define HW_TIMER_USE_TC5	1

#define HW_TIMER_INVALID	0xFF

#define HW_TIMER_SRC_CLOCK_MHZ		(VARIANT_MCK / UINT32_C(1000000))

#else
#error Target not supported!
#endif

typedef void (*HW_Timer_Callback_t)(void * param);

class HW_Timer
{
	private:
		uint8_t iTimer;		// Index of hardware timer
		uint32_t period_us;	// Current period (µs)
		
		void setPeriod(uint32_t period_us);	// Max period = 1.39808 s @ GCLK1 = 48 MHz
	
	public:
		HW_Timer(uint8_t iTimer);
		
		void init(HW_Timer_Callback_t fCallback, void * CallbackParam);
		void init(uint32_t period_us, HW_Timer_Callback_t fCallback, void * CallbackParam);
		void free();
		
		void start();
		void stop();
		
		uint32_t getPeriod() { return period_us; }
};

#endif
