/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#ifndef CONTROL_EVENT_BUFFER_h
#define CONTROL_EVENT_BUFFER_h

#include "definitions.h"

#ifndef __ARM_ARCH_6M__
#include <atomic>
#endif

struct ControlEvent
{
	enum EventType : uint8_t
	{
		NO_EVENT,
		VALUE_CHANGE,
		MOTION_TO_IDLE
	};
	
	uint8_t controlIndex;
	EventType type;
	uint16_t value;
};

// Implements a circular buffer of control events.
//   The producer (enqueue) is the code that gathers/builds control events,
//   the consumer (dequeue) is the code that handles/dispatches such events.

template <uint32_t EventBufferSize>
class ControlEventBuffer
{
public:
	ControlEventBuffer() { reset(); }
	
	uint32_t getBufferSize() { return bufferSize; }
	
	void reset()
	{
	#ifdef __ARM_ARCH_6M__
		nbEvents = 0;
	#else
		std::atomic_init(&nbEvents, 0);
	#endif
		readIndex = 0;
		writeIndex = 0;
		
		// Stats.
		nbEventsMax = 0;
	}
	
	inline uint32_t getEventsNum()
	{
	#ifdef __ARM_ARCH_6M__
		return ARM_6M_Atomic_LoadAdd(&nbEvents, 0);
	#else
		return std::atomic_load(&nbEvents);
	#endif
	}
	
	inline bool isEmpty(){ return (getEventsNum() == 0); }
	inline bool isFull(){ return (getEventsNum() == bufferSize); }
	
	inline bool enqueue(ControlEvent &newEvent)
	{
		if (isFull())
			return false;
	
		EventBuffer[writeIndex] = newEvent;
		writeIndex = (writeIndex + 1) & bufferIndexMask;
	#ifdef __ARM_ARCH_6M__
		uint32_t nbEventsPrev = ARM_6M_Atomic_LoadAdd(&nbEvents, 1);
	#else
		uint32_t nbEventsPrev = std::atomic_fetch_add(&nbEvents, 1);
	#endif
		
		// Stats.
		if ((nbEventsPrev + 1) > nbEventsMax)
			nbEventsMax = nbEventsPrev + 1;
		
		return true;
	}
	
	inline bool enqueue(uint8_t controlIndex, ControlEvent::EventType type, uint16_t value = 0)
	{
		ControlEvent newEvent = { .controlIndex = controlIndex, .type = type, .value = value };
		return enqueue(newEvent);
	}
	
	inline ControlEvent dequeue()
	{
		ControlEvent readEvent;
		
		if (! isEmpty())
		{
			readEvent = EventBuffer[readIndex];
			readIndex = (readIndex + 1) & bufferIndexMask;
		#ifdef __ARM_ARCH_6M__
			ARM_6M_Atomic_LoadAdd(&nbEvents, (uint32_t)(INT32_C(-1)));
		#else
			std::atomic_fetch_sub(&nbEvents, 1);
		#endif
		}
		else
		{
			// readEvent = { .type = ControlEvent::EventType::NO_EVENT };
			readEvent.type = ControlEvent::EventType::NO_EVENT;
		}
		
		return readEvent;
	}
	
	inline ControlEvent peek()
	{
		return (isEmpty()? (ControlEvent){ .type = ControlEvent::EventType::NO_EVENT } : EventBuffer[readIndex]);
	}
	
	// Stats.
	uint32_t getEventsNumMax() { return nbEventsMax; }
	
private:
	static constexpr uint32_t bufferSize = EventBufferSize;
	static_assert((bufferSize & (bufferSize - 1)) == 0, "ControlEventBuffer size must be a power of two!");
	static constexpr uint32_t bufferIndexMask = bufferSize - 1;
	
	std::array<ControlEvent, bufferSize> EventBuffer;
	
#ifdef __ARM_ARCH_6M__
	volatile uint32_t nbEvents;
#else
	std::atomic_uint32_t nbEvents;
#endif
	uint32_t readIndex;
	uint32_t writeIndex;
	
	uint32_t nbEventsMax; // Stats
	
#ifdef __ARM_ARCH_6M__
//#warning ControlEventBuffer on Cortex M0/M0+
	__attribute__((always_inline))
	static inline uint32_t ARM_6M_Atomic_LoadAdd(volatile uint32_t *pnDest, uint32_t nValue)
	{
		uint32_t nRs, nRd;
		
		__asm__ volatile
		(
			"cpsid i\n"			\
			"ldr %0, [%2]\n"	\
			"mov %1, %0\n"		\
			"add %0, %0, %3\n"	\
			"str %0, [%2]\n"	\
			"cpsie i\n"			\
			: "=&b"(nRs), "=&b"(nRd) : "b"(pnDest), "r"(nValue) : "cc", "memory"
		);
		
		return nRd;
	}
#endif
};

#endif
