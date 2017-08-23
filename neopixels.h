//*****************************************************************************
// NeoPixel Library for Tiva LaunchPad
// Usage: Connect data line on a NeoPixel strip to GPIO port B pin 2.
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

#define NEOPIXELS_PERIPH      SYSCTL_PERIPH_GPIOB  // For enabling clocking
#define NEOPIXELS_GPIO_BASE   GPIOB_BASE           // GPIO port base address
#define NEOPIXELS_GPIO_PIN_M  GPIO_PIN_2           // Pin mask
#define NEOPIXELS_GPIO_REG    GPIOB                // Register map
#define NUM_NEOPIXELS         150                  // Number of NeoPixels

// Primarily used to organize colors into a single argument for the 
// neopixels_set_LED_color() method
typedef struct {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} Color;

//*****************************************************************************
// Configures the GPIO pin appropriately.
//*****************************************************************************
void neopixels_config(void);

//*****************************************************************************
// Sets an LED in the strip to the desired color.
//*****************************************************************************
void neopixels_set_LED_color(uint16_t index, Color *color);

void neopixels_set_LED_wavelength(uint16_t index, double wavelength);

//*****************************************************************************
// Sends data to the LED strip following the protocol specified by the 
// datasheet. I verified timing correctness using an oscilloscope.
//*****************************************************************************
void neopixels_send_data(void);

#endif
