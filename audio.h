//*****************************************************************************
// Audio Library for Tiva LaunchPad
// Usage: Connect normalized left channel output to GPIO port E pin 3 and 
// normalized right channel output to GPIO port E pin 2.
// Author: Zachary Zhou
//*****************************************************************************

#ifndef __AUDIO_H__
#define __AUDIO_H__

#define PART_TM4C123GH6PM

#include <stdbool.h>
#include <stdint.h>
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "TM4C123.h"

//*****************************************************************************
// Configures the GPIO pins connected to the audio channels appropriately.
//*****************************************************************************
void audio_config(void);

#endif
