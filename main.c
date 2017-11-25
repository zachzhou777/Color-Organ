//*****************************************************************************
// Color Organ
// Usage: 
// Author: Zachary Zhou
//*****************************************************************************

#include "main.h"

volatile uint32_t left_audio_sample;
volatile uint32_t right_audio_sample;

// Interrupt flags
volatile bool Timer0A_flag;
volatile bool ADC0SS0_flag;
volatile bool ADC1SS0_flag;

bool left_fft_done;
bool right_fft_done;

//*****************************************************************************
// Note: Maximum frequency of music that can be detected is 10 kHz.
//*****************************************************************************
void timer_config(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 2500);
    TimerControlTrigger(TIMER0_BASE, TIMER_A, true);
    IntEnable(INT_TIMER0A);
    //TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

//*****************************************************************************
// Initialize and configure all the relevant hardware.
//*****************************************************************************
void hardware_config(void) {
    IntMasterDisable();
    
    //audio_config();
    neopixels_config(GPIOB_BASE, 2);
    print_config();
    
    IntMasterEnable();
}

//*****************************************************************************
// ISRs for analog-to-digital conversions; they simply indicate that the 
// handler has been entered and retrieve data.
//*****************************************************************************
void ADC0SS0_Handler(void) {
    ADCIntClear(ADC0_BASE, 0);
    ADC0SS0_flag = true;
    ADCSequenceDataGet(ADC0_BASE, 0, &left_audio_sample);
}

void ADC1SS0_Handler(void) {
    ADCIntClear(ADC1_BASE, 0);
    ADC1SS0_flag = true;
    ADCSequenceDataGet(ADC1_BASE, 0, &right_audio_sample);
}

uint32_t freq_band_to_color(uint16_t idx) {
    double wavelength = 780.0 - 400.0*idx/NUM_SAMPLES;
    return wavelength_to_rgb(wavelength, true);
}

uint32_t led_index_to_color(uint16_t idx) {
    double wavelength = 780.0 - 400.0*idx/NUM_NEOPIXELS;
    return wavelength_to_rgb(wavelength, true);
}

void test_neopixels1(void) {
    const uint32_t red       = 0x00FF0000;
    const uint32_t green     = 0x0000FF00;
    const uint32_t blue      = 0x000000FF;
    const uint32_t colorless = 0x00000000;
    
    uint32_t color;
    uint32_t i, j, k;
    
    hardware_config();
    
    for (i = 0; i < NUM_NEOPIXELS; i++) neopixel_data[i] = colorless;
    
    while (1) {
        for (i = 0; i < 3; i++) {
            switch (i % 3) {
                case 0:
                    color = red;
                    break;
                case 1:
                    color = green;
                    break;
                default:
                    color = blue;
            }
            for (j = 0; j < NUM_NEOPIXELS; j++) {
                for (k = 0; k < NUM_NEOPIXELS; k++) {
                    neopixel_data[k] = (k <= j) ? color : colorless;
                }
                flash_neopixels();
                
                // Wait a while
                for (k = 0; k < 10000; k++) __asm {
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
                }
            }
        }
    }
}

void test_neopixels2(bool increase_wavelength) {
    double wavelength;
    uint32_t color;
    uint32_t i;
    
    hardware_config();
    
    if (increase_wavelength) wavelength = 380.0;
    else wavelength = 780.0;
    while (1) {
        if (increase_wavelength) {
            wavelength++;
            if (wavelength > 780.0) wavelength = 380.0;
        }
        else {
            wavelength--;
            if (wavelength < 380.0) wavelength = 780.0;
        }
        
        color = wavelength_to_rgb(wavelength, true);
        for (i = 0; i < NUM_NEOPIXELS; i++) {
            neopixel_data[i] = color;
        }
        flash_neopixels();
        
        // Wait a while
        for (i = 0; i < 100000; i++) __asm {
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
        }
    }
}

void test_neopixels3(void) {
    const uint32_t colorless = 0x00000000;
    
    uint32_t color;
    uint32_t i, j, k;
    
    hardware_config();
    
    for (i = 0; i < NUM_NEOPIXELS; i++) neopixel_data[i] = colorless;
    
    while (1) {
        for (j = 0; j < NUM_NEOPIXELS; j++) {
            for (k = 0; k < NUM_NEOPIXELS; k++) {
                color = led_index_to_color(k);
                neopixel_data[k] = (k <= j) ? color : colorless;
            }
            flash_neopixels();
            
            // Wait a while
            for (k = 0; k < 10000; k++) __asm {
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
            }
        }
    }
}

int main(void) {
    test_neopixels1();
    double complex left_channel_samples[NUM_SAMPLES];
    double complex right_channel_samples[NUM_SAMPLES];
    uint16_t left_sample_num = 0;
    uint16_t right_sample_num = 0;
    double complex *left_channel_output;
    double complex *right_channel_output;
    
    // Index 0 is meaningless
    double normalized_output;
    double normalized_averages[NUM_SAMPLES/2];
    
    uint32_t num_cycles = 0;
    double curr_ratio, best_ratio;
    uint32_t color;
    uint16_t i, best_i;
    
    hardware_config();
    
    while (1) {
        if (ADC0SS0_flag) {
            ADC0SS0_flag = false;
            left_channel_samples[left_sample_num] = (double complex) left_audio_sample / 0xFFF;
            left_sample_num++;
            if (left_sample_num >= NUM_SAMPLES) {
                left_sample_num = 0;
                left_channel_output = fft(left_channel_samples);
                left_fft_done = true;
            }
        }
        
        if (ADC1SS0_flag) {
            ADC1SS0_flag = false;
            right_channel_samples[right_sample_num] = (double complex) right_audio_sample / 0xFFF;
            right_sample_num++;
            if (right_sample_num >= NUM_SAMPLES) {
                right_sample_num = 0;
                right_channel_output = fft(right_channel_samples);
                right_fft_done = true;
            }
        }
        
        if (left_fft_done && right_fft_done) {
            left_fft_done = false;
            right_fft_done = false;
            best_ratio = 0.0;
            for (i = 1; i < NUM_SAMPLES/2; i++) {
                normalized_output = cabs(left_channel_output[i]) + cabs(right_channel_output[i]);
                curr_ratio = (num_cycles) ? normalized_output/normalized_averages[i] : normalized_output;
                if (curr_ratio > best_ratio) {
                    best_ratio = curr_ratio;
                    best_i = i;
                }
                normalized_averages[i] = (num_cycles*normalized_averages[i] + normalized_output) / (num_cycles + 1);
                printf("%d: %f\n", i, normalized_averages[i]);
            }
            num_cycles++;
            
            color = freq_band_to_color(best_i);
            for (i = 0; i < NUM_NEOPIXELS; i++) {
                neopixel_data[i] = color;
            }
            flash_neopixels();
        }
    }
}
