//*****************************************************************************
// UART-based Printing Utility
// Usage: 
// Author: Zachary Zhou
//*****************************************************************************

#include "print.h"

//*****************************************************************************
// 
//*****************************************************************************
void print_config(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
	
	GPIOPinConfigure(GPIO_PA1_U0TX);
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));
	
	GPIOPinTypeUART(GPIOA_BASE, GPIO_PIN_1);
	
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, EIGHT_N_ONE);
}

//*****************************************************************************
// 
//*****************************************************************************
int fputc(int c, FILE* stream) {
	UARTCharPut(UART0_BASE, (unsigned char) c);
	if (c == '\n') UARTCharPut(UART0_BASE, '\r');
	return c;
}
