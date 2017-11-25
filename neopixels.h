//*****************************************************************************
// NeoPixel (WS2812B) Library for Tiva LaunchPad
// Usage: Configure the desired GPIO pin for driving a NeoPixels. If needed, 
//   convert desired frequencies of light on the visible spectrum to 24-bit 
//   RGB codes. Connect the NeoPixels' data line to the specified pin to flash 
//   them.
// Author: Zachary Zhou
//*****************************************************************************

#ifndef __NEOPIXELS_H__
#define __NEOPIXELS_H__

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "TM4C123.h"

#define NUM_NEOPIXELS 150  // Number of NeoPixels

extern uint32_t neopixel_data[NUM_NEOPIXELS];

//*****************************************************************************
// Configures the GPIO pin appropriately.
//*****************************************************************************
void neopixels_config(uint32_t new_gpio_base, uint8_t pin_number);

//*****************************************************************************
// Converts a wavelength in nanometers to its corresponding 24-bit RGB code. 
// Algorithm based on Dan Bruton's.
//*****************************************************************************
uint32_t wavelength_to_rgb(double wavelength, bool lookup);

//*****************************************************************************
// Sends data to the LED strip following the protocol specified by the 
// datasheet. I verified timing correctness using an oscilloscope.
//*****************************************************************************
void flash_neopixels(void);

#endif
