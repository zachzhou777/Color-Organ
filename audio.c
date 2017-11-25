//*****************************************************************************
// Audio Library for Tiva LaunchPad
// Usage: Connect normalized left channel output to GPIO port E pin 3 and 
// normalized right channel output to GPIO port E pin 2.
// Author: Zachary Zhou
//*****************************************************************************

#include "audio.h"

//*****************************************************************************
// Configures the GPIO pins connected to the audio channels, an ADC module, 
// and a sample sequencer.
//*****************************************************************************
void audio_config(void) {
    // Enable the peripherals, then wait for them to be ready
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0) || 
           !SysCtlPeripheralReady(SYSCTL_PERIPH_ADC1) || 
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE) || 
           !SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    
    // Enable SS0 to capture the values of channels 0 and 1 when timer 
    // triggers occur TODO FIX COMMENT
    ADCSequenceDisable(ADC0_BASE, 0);
    ADCHardwareOversampleConfigure(ADC0_BASE, 16);
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_TIMER, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);
    ADCSequenceEnable(ADC0_BASE, 0);
    IntEnable(INT_ADC0SS0);
    ADCIntEnable(ADC0_BASE, 0);
    
    // Enable 
    ADCSequenceDisable(ADC1_BASE, 0);
    ADCHardwareOversampleConfigure(ADC1_BASE, 16);
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_TIMER, 0);
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH1);
    ADCSequenceEnable(ADC1_BASE, 0);
    IntEnable(INT_ADC1SS0);
    ADCIntEnable(ADC1_BASE, 0);
    
    // Configure pins as ADC inputs
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / (2 * 5000));
    TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
    IntEnable(INT_TIMER0A);
    //TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);
}
