//*****************************************************************************
// NeoPixel Library for Tiva LaunchPad
// Usage: Connect data line on a NeoPixel strip to GPIO port B pin 2.
// Author: Zachary Zhou
//*****************************************************************************

#include "neopixels.h"

// Array stores the color of each LED as a 24-bit RGB code
static uint32_t data[NUM_NEOPIXELS];

//*****************************************************************************
// Disables interrupts.
//*****************************************************************************
__inline static void disable_interrupts(void) {
	__asm{CPSID i}
}

//*****************************************************************************
// Enables interrupts.
//*****************************************************************************
__inline static void enable_interrupts(void) {
	__asm{CPSIE i}
}

//*****************************************************************************
// Configures the GPIO pin connected to the NeoPixels appropriately.
//*****************************************************************************
void neopixels_config(void) {
	// Enable the GPIOB peripheral, then wait for it to be ready
	SysCtlPeripheralEnable(NEOPIXELS_PERIPH);
	while (!SysCtlPeripheralReady(NEOPIXELS_PERIPH));
	
	// Configure pin as digital output
	GPIOPinTypeGPIOOutput(NEOPIXELS_GPIO_BASE, NEOPIXELS_GPIO_PIN_M);
}

//*****************************************************************************
// Sets an LED in the strip to the desired color.
//*****************************************************************************
void neopixels_set_LED_color(uint16_t index, Color *color) {
    // Set the appropriate integer element; note that the upper 8 bits are 
    // unmodified
    data[index] = (color->green << 16) + (color->red << 8) + color->blue;
}

//*****************************************************************************
// Converts a wavelength in nanometers to its corresponding GRB code, i.e., 
// the 24-bit RGB code where the red and green bits are swapped.
//*****************************************************************************
static uint32_t wavelength_to_grb(double wavelength) {
    double red, green, blue, factor;
    if (wavelength < 380.0) {
        red = 0.0;
        green = 0.0;
        blue = 0.0;
    }
    else if (wavelength < 440.0) {
        red = (440.0 - wavelength) / (440.0 - 380.0);
        green = 0.0;
        blue = 1.0;
    }
    else if (wavelength < 490.0) {
        red = 0.0;
        green = (wavelength - 440.0) / (490.0 - 440.0);
        blue = 1.0;
    }
    else if (wavelength < 510.0) {
        red = 0.0;
        green = 1.0;
        blue = (510.0 - wavelength) / (510.0 - 490.0);
    }
    else if (wavelength < 580.0) {
        red = (wavelength - 510.0) / (580.0 - 510.0);
        green = 1.0;
        blue = 0.0;
    }
    else if (wavelength < 645.0) {
        red = 1.0;
        green = (645.0 - wavelength) / (645.0 - 580.0);
        blue = 0.0;
    }
    else if (wavelength < 780.0) {
        red = 1.0;
        green = 0.0;
        blue = 0.0;
    }
    else {
        red = 0.0;
        green = 0.0;
        blue = 0.0;
    }

//    if (wavelength > 700.0) factor = 0.3 + (0.7 * (780.0 - wavelength) / (780.0 - 700.0));
//    else if (wavelength < 420.0) factor = 0.3 + (0.7 * (wavelength - 380.0) / (420.0 - 380.0));
//    else factor = 0.0;

    if (wavelength < 380.0) factor = 0.0;
    else if (wavelength < 420.0) factor = 0.3 + (0.7 * (wavelength - 380.0) / (420.0 - 380.0));
    else if (wavelength < 700.0) factor = 1.0;
    else if (wavelength < 780.0) factor = 0.3 + (0.7 * (780.0 - wavelength) / (780.0 - 700.0));
    else factor = 0.0;


    //return ((((uint8_t) (green * 0xFF)) << 16) | (((uint8_t) (red * 0xFF)) << 8) | ((uint8_t) (blue * 0xFF)));
    return ((((uint8_t) (pow(factor * green, 0.8) * 0xFF)) << 16) |
            (((uint8_t) (pow(factor * red, 0.8) * 0xFF)) << 8) |
            ((uint8_t) (pow(factor * blue, 0.8) * 0xFF)));
}

//*****************************************************************************
// Sets an LED in the strip to the desired color.
//*****************************************************************************
void neopixels_set_LED_wavelength(uint16_t index, double wavelength) {
    data[index] = wavelength_to_grb(wavelength);
}

//*****************************************************************************
// Halts execution for one microsecond.
//*****************************************************************************
static void wait_microsecond(void) {
	// Clock rate is 50 MHz, so waiting 50 clock cycles will be equivalent to 
	// waiting one microsecond
	__asm {
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP	// 10 cycles
		
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP	// 20 cycles
		
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP	// 30 cycles
		
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP	// 40 cycles
		
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP	// 50 cycles
	}
}

//*****************************************************************************
// Sends data to the LED strip following the protocol specified by the 
// datasheet. I verified timing correctness using an oscilloscope.
//*****************************************************************************
void neopixels_send_data(void) {
	// Get the address of the GPIO data register
	uint32_t gpio_addr = (uint32_t) (void *) &(NEOPIXELS_GPIO_REG->DATA);
	
	// Loop counters; one is unsigned and the other is signed intentionally
	uint8_t i;
	int8_t j;
	
	// Must disable interrupts before transmission due to timing constraints
	disable_interrupts();
	
	// 'high' and 'low' hold values to be stored in the GPIO data register when 
	// driving the pin high or low; 'bit' will be assigned one of these values
	const uint32_t high = NEOPIXELS_GPIO_REG->DATA | NEOPIXELS_GPIO_PIN_M;
	const uint32_t low = NEOPIXELS_GPIO_REG->DATA & ~NEOPIXELS_GPIO_PIN_M;
	uint32_t bit;
	
	// Transfer data through the GPIO pin; since it's tricky to predict how long 
	// the code will take to execute (especially since some of it's C), I used 
	// an oscilloscope to tweak the number of NOPs needed
	for (i = 0; i < NUM_NEOPIXELS; i++) {
		for (j = 23; j >= 0; j--) {
			bit = (data[i] & (1 << j)) ? high : low;
			__asm {
				// Drive pin high
				STR high, [gpio_addr]
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				// Pull pin low
				STR bit, [gpio_addr]
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				NOP
				// If the pin wasn't pulled low before, it will be now
				STR low, [gpio_addr]
				NOP
				NOP
			}
		}
	}
	
	// Done transmitting data, so reenable interrupts
	enable_interrupts();
	
	// Wait >50 µs
	for (i = 0; i < 50; i++) wait_microsecond();
}
/*
void neopixels_send_data(void) {
	uint32_t i;
	
	uint32_t data_addr = (uint32_t) data;
	
	// Get the address of the GPIO data register
	uint32_t gpio_addr = (uint32_t) (void *) &(NEOPIXELS_GPIO_REG->DATA);
	
	uint32_t led_data;
	
	uint32_t mask;
	
	// Loop counters
	uint32_t num_leds_remaining = NUM_NEOPIXELS;
	uint32_t num_bits_remaining = 8;
	uint32_t num_colors_remaining = 3;
	
	// Must disable interrupts before transmission due to timing constraints
	disable_interrupts();
	
	// 'high' and 'low' hold values to be stored in the GPIO data register when 
	// driving the pin high or low; 'bit' will be assigned one of these values
	const uint32_t high = NEOPIXELS_GPIO_REG->DATA | NEOPIXELS_GPIO_PIN_M;
	const uint32_t low = NEOPIXELS_GPIO_REG->DATA & ~NEOPIXELS_GPIO_PIN_M;
	uint32_t bit;
	
	__asm {
	led_loop_start:
		LDR led_data, [data_addr], 4
	color_loop_start:
		MOV mask, 0x0080
		CMP num_leds_remaining, 2
		MOVEQ mask, 0x8000
		CMP num_leds_remaining, 1
		MOVEQ mask, 0x0800
	bits_loop_start:
		STR high, [gpio_addr]
		TST led_data, mask
		MOV bit, high
		MOVEQ bit, low
		LSL led_data, led_data, 1
		
		STR bit, [gpio_addr]
		NOP
		NOP
		STR low, [gpio_addr]
		NOP
		NOP
		SUBS num_leds_remaining, num_leds_remaining, 1
		BNE led_loop_start
		SUBS num_colors_remaining, num_colors_remaining, 1
		BNE color_loop_start
		NOP
		NOP
		NOP
		NOP
		SUBS num_bits_remaining, num_bits_remaining, 1
		BNE bits_loop_start
	}
	
	// Done transmitting data, so reenable interrupts
	enable_interrupts();
	
	// Wait >50 µs
	for (i = 0; i < 50; i++) wait_microsecond();
}*/
