//*****************************************************************************
// Audio Library for Tiva LaunchPad
// Usage: Connect normalized left channel output to GPIO port E pin 3 and 
// normalized right channel output to GPIO port E pin 2.
// Author: Zachary Zhou
//*****************************************************************************

#include "audio.h"

static uint32_t buffer[2];

//*****************************************************************************
// Configures the GPIO pins connected to the audio channels, an ADC module, 
// and a sample sequencer.
//*****************************************************************************
void audio_config(void) {
	// Enable the GPIOE peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	
	// Wait for the GPIOE module to be ready
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE));
	
	// Configure pins as ADC inputs
	GPIOPinTypeADC(GPIOE_BASE, GPIO_PIN_2 | GPIO_PIN_3);
	
	// Enable the ADC0 module
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	
	// Wait for the ADC0 module to be ready
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0));
	
	// Enable SS0 to capture the values of channels 0 and 1 when processor 
	// triggers occur
	ADCSequenceDisable(ADC0_BASE, 0);
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH1);
	ADCSequenceEnable(ADC0_BASE, 0);
}

//*****************************************************************************
// Triggers a sample sequence and reads values from ADC into 'buffer' for 
// later retrieval.
//*****************************************************************************
void audio_get_data(void) {
	// Trigger sampling
	ADCProcessorTrigger(ADC0_BASE, 0);
	
	// Wait until sample sequence has completed
	while (!ADCIntStatus(ADC0_BASE, 0, false));
	
	// Read value from ADC
	ADCSequenceDataGet(ADC0_BASE, 0, buffer);
}

//*****************************************************************************
// Returns the last value obtained by the ADC at the left audio channel.
//*****************************************************************************
uint32_t audio_left_channel(void) {
	return buffer[0];
}

//*****************************************************************************
// Returns the last value obtained by the ADC at the right audio channel.
//*****************************************************************************
uint32_t audio_right_channel(void) {
	return buffer[1];
}
