/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "devices.h"

/* Device setup data */
Device_t device;
CustomADC customADC;
ADC_MUX muxFactory(device.pots, customADC, device.potsEventBuffer);
