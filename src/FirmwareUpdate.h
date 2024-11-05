#ifndef FIRMWARE_UPDATE_h
#define FIRMWARE_UPDATE_h

// Firmware Update Utilities.
// These utilities are meant to be used along with the provided UF2 Bootloader.
//
// The linker script reserves the last 32-bit word in RAM (highest address) to be shared
// between the bootloader and user applications, so one can use it to force entering
// the bootloader into Firmware Update mode directly.

#ifdef __cplusplus
extern "C" {
#endif

#include "variant.h"

// Address of the shared word in RAM.
#ifdef _SAMD21_
#define BL_DBL_TAP_PTR					((volatile uint32_t *)(HMCRAMC0_ADDR + HMCRAMC0_SIZE - 4))
#elif defined(_SAMD51_)
#define BL_DBL_TAP_PTR					((volatile uint32_t *)(HSRAM_ADDR + HSRAM_SIZE - 4))
#else
#warning Firmware Update: unsupported target.
#endif

// Magic values used in the bootloader.
#define BL_DBL_TAP_MAGIC				0xf01669ef
#define BL_DBL_TAP_MAGIC_QUICK_BOOT		0xf02669ef

// To be called at initialization of firmware to avoid the magic value
// to be unexpectedly set at the end of RAM (until we explicitely call
// FirmwareUpdate_EnterBootloader()), which would cause entering the
// bootloader in case the firmware resets the MCU for a different reason.
//
static inline void FirmwareUpdate_Init(void)
{
	*BL_DBL_TAP_PTR = 0;
}

// To be called for entering the bootloader in Firmware Update mode.
//
static inline void FirmwareUpdate_EnterBootloader(void)
{
	*BL_DBL_TAP_PTR = BL_DBL_TAP_MAGIC;
	NVIC_SystemReset();
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif
