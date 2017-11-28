//*****************************************************************************
// NeoPixel (WS2812B) Library for Tiva LaunchPad
// Usage: Configure the desired GPIO pin for driving a NeoPixels. If needed, 
//   convert desired frequencies of light on the visible spectrum to 24-bit 
//   RGB codes. Connect the NeoPixels' data line to the specified pin to flash 
//   them.
// Author: Zachary Zhou
//*****************************************************************************

#include "neopixels.h"
#include "print.h"

// Set by neopixels_config(); used as arguments for flash_neopixels()
static uint32_t gpio_base;
static uint8_t pin_mask;

// Array stores the color of each LED as a 24-bit RGB code; the upper byte is 
// ignored, and the remaining 24 bits are the RGB code
uint32_t neopixel_data[NUM_NEOPIXELS];
static uint32_t dud[NUM_NEOPIXELS];

//*****************************************************************************
// Configures the GPIO pin connected to the NeoPixels appropriately.
//*****************************************************************************
void neopixels_config(uint32_t new_gpio_base, uint8_t pin_number) {
    // Set global variables
    gpio_base = new_gpio_base;
    pin_mask = 1 << pin_number;
    
    // Values used as arguments for function calls
    uint32_t sysctl_periph_value;
    uint32_t pin_value;
    
    // Set 'sysctl_periph_value' based on 'gpio_base' argument; returns if value 
    // of 'gpio_base' is invalid
    switch (gpio_base) {
        case GPIOA_BASE:
            sysctl_periph_value = SYSCTL_PERIPH_GPIOA;
            break;
        case GPIOB_BASE:
            sysctl_periph_value = SYSCTL_PERIPH_GPIOB;
            break;
        case GPIOC_BASE:
            sysctl_periph_value = SYSCTL_PERIPH_GPIOC;
            break;
        case GPIOD_BASE:
            sysctl_periph_value = SYSCTL_PERIPH_GPIOD;
            break;
        case GPIOE_BASE:
            sysctl_periph_value = SYSCTL_PERIPH_GPIOE;
            break;
        case GPIOF_BASE:
            sysctl_periph_value = SYSCTL_PERIPH_GPIOF;
            break;
        default:
            return;
    }
    
    // Set 'pin_value' based on 'pin_number' argument; returns if value of 
    // 'pin_number' is invalid
    // Note: I realize that 'pin_value' is the same as 'pin_mask' given macro 
    // definitions, but in the event the macros are defined differently in the 
    // future, I will assign 'pin_value' using the macros as follows
    switch (pin_number) {
        case 0:
            pin_value = GPIO_PIN_0;
            break;
        case 1:
            pin_value = GPIO_PIN_1;
            break;
        case 2:
            pin_value = GPIO_PIN_2;
            break;
        case 3:
            pin_value = GPIO_PIN_3;
            break;
        case 4:
            pin_value = GPIO_PIN_4;
            break;
        case 5:
            pin_value = GPIO_PIN_5;
            break;
        case 6:
            pin_value = GPIO_PIN_6;
            break;
        case 7:
            pin_value = GPIO_PIN_7;
            break;
        default:
            return;
    }
    
    // Enable the GPIOB peripheral, then wait for it to be ready
    SysCtlPeripheralEnable(sysctl_periph_value);
    while (!SysCtlPeripheralReady(sysctl_periph_value));
    
    // Configure pin as digital output
    GPIOPinTypeGPIOOutput(gpio_base, pin_value);
}

//*****************************************************************************
// Converts a wavelength in nanometers to its corresponding 24-bit RGB code. 
// Algorithm based on Dan Bruton's.
//*****************************************************************************
static uint32_t wavelength_to_rgb_helper(double wavelength) {
    const double GAMMA = 0.8;
    double red, green, blue, factor;
    
    if (wavelength < 380.0) {
        red = 0.0;
        green = 0.0;
        blue = 0.0;
    }
    else if (wavelength < 440.0) {
        red = (440.0 - wavelength)/(440.0 - 380.0);
        green = 0.0;
        blue = 1.0;
    }
    else if (wavelength < 490.0) {
        red = 0.0;
        green = (wavelength - 440.0)/(490.0 - 440.0);
        blue = 1.0;
    }
    else if (wavelength < 510.0) {
        red = 0.0;
        green = 1.0;
        blue = (510.0 - wavelength)/(510.0 - 490.0);
    }
    else if (wavelength < 580.0) {
        red = (wavelength - 510.0)/(580.0 - 510.0);
        green = 1.0;
        blue = 0.0;
    }
    else if (wavelength < 645.0) {
        red = 1.0;
        green = (645.0 - wavelength)/(645.0 - 580.0);
        blue = 0.0;
    }
    else if (wavelength <= 780.0) {
        red = 1.0;
        green = 0.0;
        blue = 0.0;
    }
    else {
        red = 0.0;
        green = 0.0;
        blue = 0.0;
    }
        
    if (wavelength < 380.0)
        factor = 0.0;
    else if (wavelength < 420.0)
        factor = 0.3 + 0.7*(wavelength - 380.0)/(420.0 - 380.0);
    else if (wavelength < 700.0)
        factor = 1.0;
    else if (wavelength <= 780.0)
        factor = 0.3 + 0.7*(780.0 - wavelength)/(780.0 - 700.0);
    else
        factor = 0.0;
    
    return (((uint8_t) (pow(factor * red, GAMMA)*0xFF)) << 16) |
            (((uint8_t) (pow(factor * green, GAMMA)*0xFF)) << 8) |
            ((uint8_t) (pow(factor * blue, GAMMA)*0xFF));
}

uint32_t wavelength_to_rgb(double wavelength, bool lookup) {
    static uint32_t lut[401];
    uint16_t idx;
    
    if (!lookup) return wavelength_to_rgb_helper(wavelength);
    
    if ((wavelength < 380.0) || (wavelength > 780.0)) return 0x00000000;
    
    wavelength = round(wavelength);
    idx = wavelength - 380;
    if (lut[idx]) return lut[idx];
    lut[idx] = wavelength_to_rgb_helper(wavelength);
    return lut[idx];
}

//*****************************************************************************
// EABI compliant wrapper function that calls send_neopixels_data().
//*****************************************************************************
void flash_neopixels(void) {
    // Save 'gpio_base' as it is overwritten in send_neopixels_data()
    uint32_t temp = gpio_base;
    send_neopixels_data(
        gpio_base + 255*sizeof(uint32_t),  // GPIO data register address
        pin_mask,                          // GPIO pin mask
        (uint32_t) neopixel_data,          // Data array address
        NUM_NEOPIXELS                      // Number of NeoPixels
    );
    gpio_base = temp;
}

void clear_neopixels(void) {
    uint32_t temp = gpio_base;
    send_neopixels_data(
        gpio_base + 255*sizeof(uint32_t),  // GPIO data register address
        pin_mask,                          // GPIO pin mask
        (uint32_t) dud,                    // Dud array address
        NUM_NEOPIXELS                      // Number of NeoPixels
    );
    gpio_base = temp;
}
