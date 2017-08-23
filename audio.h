//*****************************************************************************
// Audio Library for Tiva LaunchPad
// Usage: Connect normalized left channel output to GPIO port E pin 3 and 
// normalized right channel output to GPIO port E pin 2.
// Author: Zachary Zhou
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "TM4C123.h"

//*****************************************************************************
// Configures the GPIO pins connected to the audio channels appropriately.
//*****************************************************************************
void audio_config(void);

//*****************************************************************************
// Triggers a sample sequence and reads values from ADC into 'buffer' for 
// later retrieval.
//*****************************************************************************
void audio_get_data(void);

__inline uint32_t audio_left_channel(void);

__inline uint32_t audio_right_channel(void);
