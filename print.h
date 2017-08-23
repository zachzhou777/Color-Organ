//*****************************************************************************
// Audio Library for Tiva LaunchPad
// Usage: Connect normalized left channel output to GPIO port E pin 3 and 
// normalized right channel output to GPIO port E pin 2.
// Author: Zachary Zhou
//*****************************************************************************
#ifndef __PRINT_H__
#define __PRINT_H__

#define PART_TM4C123GH6PM

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "TM4C123.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"

#define EIGHT_N_ONE (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE)

//*****************************************************************************
// 
//*****************************************************************************
void print_config(void);

//*****************************************************************************
// 
//*****************************************************************************
int fputc(int c, FILE* stream);

#endif
