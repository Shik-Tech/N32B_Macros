/*
  N32B Macros Firmware v4.x.x
  MIT License

  Copyright (c) 2024 SHIK
*/

#include "functions.h"
#include <GlobalComponents/GlobalComponents.h>
#include <FirmwareUpdate.h>
#include <wiring_private.h>

static void setDefaultPinFunction(uint8_t pin);
static void enablePowerSwitch(bool bOn);
static void handleUsbSuspend();
static void dualLEDon(bool bOn);
static void ADC_Callback(uint16_t sample, void * param);

bool isSuspended = false;
bool isAddressedUSB = false;

void setup()
{
  FirmwareUpdate_Init();
  
  pinMode(PWR_SW_EN, OUTPUT);
  
  enablePowerSwitch(true);

  customADC.init(ADC_SamplingCycles, ADC_AvgSamples, ADC_Callback, &muxFactory);
  
  display.on();

  /* Pin setup */
  pinMode(MIDI_TX_LED, OUTPUT);
  pinMode(MIDI_RX_LED, OUTPUT);
  digitalWrite(MIDI_TX_LED, HIGH);
  digitalWrite(MIDI_RX_LED, HIGH);
  
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);

  /* Init. EEPROM access */
  storageInit();
  
  /*
   * Factory Reset
   * Hold button-A down while powering the device will reset the presets
   */
  if (!digitalRead(BUTTON_A))
  {
    bool buttonPressed = true;
    dualLEDon(true);

    while (millis() < reset_timeout) // Check if button has been released before timeout
    {
      if (digitalRead(BUTTON_A))
      {
        buttonPressed = false;
        break;
      }
    }

    // If button is still held down, then clear eeprom
    if (buttonPressed)
    {
      // Blink once if reset request has been accepted
      dualLEDon(false);
      delay(20);
      dualLEDon(true);

      // Clean eeprom
      for (unsigned int i = 0; i < EEPROM_Size; i++)
      {
        EEPROM.write(i, 0);
      }
    }
    
    dualLEDon(false);
  }

  // Write the factory presets to memory if the device was turn on for the first time
  if (!isEEPROMvalid())
  {
    for (int i = 0; i < NUMBER_OF_PRESETS; i++)
    {
      dualLEDon(true);
      delay(300);
      dualLEDon(false);
      delay(300);
    }
    display.factoryResetAnimation();
    formatFactory();
  }

  //muxFactory.init();

  // Load the last used preset as stored in EEPROM
  loadPreset(EEPROM.read(lastUsedPresetAddress));

  /* Set callbacks */
  MIDIUSB.setHandleMessage(onUsbMessage);
  MIDISerial1.setHandleMessage(onSerialMessage);

  MIDIUSB.setHandleSystemExclusive(processSysex);
  MIDISerial1.setHandleSystemExclusive(processSysex);

  MIDIUSB.setHandleProgramChange(handleProgramChange);
  MIDISerial1.setHandleProgramChange(handleProgramChange);

  /* Initiate MIDI communications, listen to all channels */
  MIDIUSB.begin(MIDI_CHANNEL_OMNI);
  MIDISerial1.begin(MIDI_CHANNEL_OMNI);

  MIDIUSB.turnThruOff();
  MIDISerial1.turnThruOff();

  display.showStartUpAnimation();
  
  // Start ADC acquisitions.
  muxFactory.triggerADC(0);
}

void loop()
{
  if (!isSuspended)
  {
    for (uint8_t currentKnob = 0; currentKnob < NUMBER_OF_KNOBS; currentKnob++)
    {
      muxFactory.update(currentKnob);
    }

    display.resetChanged();

    for (uint8_t currentKnob = 0; currentKnob < NUMBER_OF_KNOBS; currentKnob++)
    {
      updateKnob(currentKnob);
    }

    doMidiRead();

    handleButtons();
    display.clearDisplay();

    if (display.hasChanged())
      delayMicroseconds(1000);
  }
  
  handleUsbSuspend();
}

// ADC Callback from ADC interrupt, called when a sample conversion is completed.
//
static void ADC_Callback(uint16_t sample, void * param)
{
  ADC_MUX * MuxFactoryPtr = (ADC_MUX *) param;

  if (MuxFactoryPtr != nullptr)
  {
    MuxFactoryPtr->update(sample);
  }
}

// Enable or disable the 3V3_SW power rail.
// * When disabling it, set all pins used as outputs to low level to avoid any of them
//   powering board peripherals (normally powered via this rail) via internal diode clamps.
//   It's not necessary for the pins that directly control LEDs.
// * When enabling it, restore the function of these pins.
// This function may need to be updated when designing new Control boards.
//
static void enablePowerSwitch(bool bOn)
{
    if (bOn)
    {
      #ifdef N32B_SAMD21_ON_S32
        // Enable the 3V3_SW power rail.
        digitalWrite(PWR_SW_EN, LOW);
      #else
        // Enable the 3V3_SW / 5W_SW power rails.
        digitalWrite(PWR_SW_EN, HIGH);
      #endif
        
        // Restore pins function.
        setDefaultPinFunction(MIDI_TX);
        
        setDefaultPinFunction(DISP_SIN);
        setDefaultPinFunction(DISP_SCLK);
        setDefaultPinFunction(DISP_LAT);
        setDefaultPinFunction(DISP_BLANK);
        digitalWrite(DISP_BLANK, LOW);
    }
    else
    {
        // Set all output pins to a low level. Not required for the LEDs.
        digitalWrite(MIDI_TX, LOW);
        pinMode(MIDI_TX, OUTPUT);
        
        digitalWrite(AN_MUX_S0, LOW);
        digitalWrite(AN_MUX_S1, LOW);
        digitalWrite(AN_MUX_S2, LOW);
        digitalWrite(AN_MUX_S3, LOW);
        
        digitalWrite(DISP_SIN, LOW);
        digitalWrite(DISP_SCLK, LOW);
        digitalWrite(DISP_LAT, LOW);
        digitalWrite(DISP_BLANK, LOW);
        pinMode(DISP_SIN, OUTPUT);
        pinMode(DISP_SCLK, OUTPUT);
        pinMode(DISP_LAT, OUTPUT);
        pinMode(DISP_BLANK, OUTPUT);
        
      #ifdef N32B_SAMD21_ON_S32
        // Disable the 3V3_SW power rail.
        digitalWrite(PWR_SW_EN, HIGH);
      #else
        // Disable the 3V3_SW / 5W_SW power rails.
        digitalWrite(PWR_SW_EN, LOW);
      #endif
    }
}

// Handle USB Suspend / Resume.
// * Disables the 3V3_SW power rail if a USB suspend condition has been detected
//   to minimize power draw when the host is in standby.
// * Re-enables it when a USB resume has been detected.
//
static void handleUsbSuspend()
{
    if (USB->DEVICE.INTFLAG.reg & USB_DEVICE_INTFLAG_SUSPEND)
    {
        // A bus suspend has been detected.
        // Clear flags.
        USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_SUSPEND | USB_DEVICE_INTFLAG_WAKEUP;
        
        if (isAddressedUSB)
        {
            // This flag indicates that the host has attributed an address to the device,
            // which is our best guess here as to whether it has successfully enumerated.
            // (See DS 32.6.2.1, Initialization.)
            // If it hasn't enumerated, then we don't do anything - this will avoid
            // spurious disabling/enabling cycles while the USB data lines are not stable
            // (for instance when plugging the device in) and should also prevent it from
            // disabling the 3V3_SW rail when the device is powered via a USB charger.
            if (! isSuspended)
            {
                // Disable 3V3_SW.
                enablePowerSwitch(false);
                
                isSuspended = true;
            }
        }
    }
    else if (USB->DEVICE.INTFLAG.reg & USB_DEVICE_INTFLAG_WAKEUP)
    {
        // A bus resume (wake-up) has been detected.
        // Clear flags.
        USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_SUSPEND | USB_DEVICE_INTFLAG_WAKEUP;
        
        if (isSuspended)
        {
            // Enable 3V3_SW.
            enablePowerSwitch(true);
            
            delay(10);
            
            isSuspended = false;
        }
    }
    
    // USB Device: USB->DEVICE.DADD.bit.ADDEN: This bit set indicates that the host
    // has attributed an address to the device (See DS 32.6.2.1, Initialization.)
    if (isAddressedUSB)
    {
        if (USB->DEVICE.DADD.bit.ADDEN == 0)
            isAddressedUSB = false;
    }
    else
    {
        if (USB->DEVICE.DADD.bit.ADDEN == 1)
        {
            USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_SUSPEND;
            isAddressedUSB = true;
        }
    }
}

// Set the given pin (as defined in enum PinIndices) to its function defined in g_APinDescription.
// This function can be used for restoring the function of a given pin after having set it to
// a different function (such as switching from GPIO to peripheral, or conversely).
//
static void setDefaultPinFunction(uint8_t pin)
{
    pinPeripheral(pin, g_APinDescription[pin].ulPinType);
}

static void dualLEDon(bool bOn)
{
  if (bOn)
  {
    digitalWrite(MIDI_TX_LED, LOW);
    digitalWrite(MIDI_RX_LED, LOW);
  }
  else
  {
    digitalWrite(MIDI_TX_LED, HIGH);
    digitalWrite(MIDI_RX_LED, HIGH);
  }
}
