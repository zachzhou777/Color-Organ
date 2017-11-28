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
    TimerLoadSet(TIMER0_BASE, TIMER_A, 5000);
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
    
    audio_config();
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

double freq_band_to_wavelength(uint16_t idx) {
    return 780.0 - 400.0*idx/(NUM_SAMPLES/2);
}

double led_index_to_wavelength(uint16_t idx) {
    return 380.0 + 400.0*idx/NUM_NEOPIXELS;
}

uint32_t freq_band_to_rgb(uint16_t idx) {
    return wavelength_to_rgb(freq_band_to_wavelength(idx), true);
}

uint32_t led_index_to_rgb(uint16_t idx) {
    return wavelength_to_rgb(led_index_to_wavelength(idx), true);
}

bool music_playing(double complex *fft_output) {
    const double EPSILON = 0.05;
    uint8_t gt_epsilon = 0;
    uint16_t i;
    for (i = 1; i < NUM_SAMPLES/2; i++) {
        if (cabs(fft_output[i]) > EPSILON) {
            printf("%d: %f\n", i, cabs(fft_output[i]));
            gt_epsilon++;
            if (gt_epsilon >= NUM_SAMPLES/8.0) {
                return true;
            }
        }
        printf("%d: %f\n", i, cabs(fft_output[i]));
    }
    return false;
}

//*****************************************************************************
// Split the NeoPixels into three groups: red, green, and blue.
//*****************************************************************************
void configure_neopixel_data(uint16_t *indices) {
    
}

/*
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
//*/
/*
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
//*/
/*
void test_neopixels3(void) {
    const uint32_t colorless = 0x00000000;
    
    uint32_t color;
    uint32_t i, j;
    
    hardware_config();
    
    for (i = 0; i < NUM_NEOPIXELS; i++) neopixel_data[i] = colorless;
    
    while (1) {
        for (i = 0; i < NUM_NEOPIXELS; i++) {
            for (j = 0; j < NUM_NEOPIXELS; j++) {
                color = led_index_to_rgb(j);
                neopixel_data[j] = (j <= i) ? color : colorless;
            }
            flash_neopixels();
            
            // Wait a while
            for (j = 0; j < 10000; j++) __asm {
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
//*/

int main(void) {
    double complex left_channel_samples[NUM_SAMPLES];
    double complex right_channel_samples[NUM_SAMPLES];
    uint16_t left_sample_num = 0;
    uint16_t right_sample_num = 0;
    double complex *left_channel_output;
    double complex *right_channel_output;
    
    // Index 0 is meaningless
    double normalized_output;
    double normalized_averages[NUM_SAMPLES/2];
    double ratios[NUM_SAMPLES/2];
    
    uint16_t best_bands[NUM_BANDS];
    double best_ratios[NUM_BANDS];
    
    uint16_t restore_indices[NUM_BANDS];
    
    double led_wavelengths[NUM_NEOPIXELS];
    
    uint16_t dead_ctr = 0;
    
    uint32_t num_cycles = 0;
    double band_color, led_index_color;
    double difference, best_difference;
    uint16_t i;
    uint8_t j, k;
    
    int times_expressed[NUM_SAMPLES/2];//
    
    hardware_config();
    
    //for (i = 0; i < NUM_NEOPIXELS; i++) neopixel_data[i] = led_index_to_rgb(i);
    
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
            
            if (!music_playing(left_channel_output) && !music_playing(right_channel_output)) {
                dead_ctr++;
                if (dead_ctr > 50) {
                    clear_neopixels();
                    dead_ctr = 50;
                }
                else {
                    for (i = 0; i < NUM_NEOPIXELS; i++) neopixel_data[i] = led_index_to_rgb(i);
                    flash_neopixels();
                }
                continue;
            }
            
            dead_ctr = 0;
            
            for (i = 0; i < NUM_NEOPIXELS; i++) neopixel_data[i] = led_index_to_rgb(i);
            
            for (i = 1; i < NUM_SAMPLES/2; i++) {
                // Take the sum of the corresponding FFT outputs to be the "normalized output"
                normalized_output = cabs(left_channel_output[i]) + cabs(right_channel_output[i]);
                
                // Update rolling averages
                normalized_averages[i] = (num_cycles*normalized_averages[i] + normalized_output) / (num_cycles + 1);
                
                // Compare each normalized output to its average
                ratios[i] = normalized_output / normalized_averages[i];
            }
            num_cycles++;
            
            // Initialize 'best_bands' and 'best_ratios' for current iteration
            for (i = 0; i < NUM_BANDS; i++) best_ratios[i] = -1.0;
            
            for (i = 1; i < NUM_SAMPLES/2; i++) {
                for (j = 0; j < NUM_BANDS; j++) {
                    if (ratios[i] > best_ratios[j]) {
                        for (k = NUM_BANDS - 1; k > j; k--) {
                            best_bands[k] = best_bands[k - 1];
                            best_ratios[k] = best_ratios[k - 1];
                        }
                        best_bands[j] = i;
                        best_ratios[j] = ratios[i];
                        break;
                    }
                }
            }
            
            /*for (i = 0; i < NUM_BANDS; i++) {
                times_expressed[best_bands[i]]++;
            }
            for (i = 1; i < NUM_SAMPLES/2; i++) {
                printf("%d: %d\n", i, times_expressed[i]);
            }
            */
            
            for (i = 0; i < NUM_BANDS; i++) {
                best_difference = 400.0;
                band_color = freq_band_to_wavelength(best_bands[i]);
                for (j = 0; j < NUM_NEOPIXELS; j++) {
                    led_index_color = led_index_to_wavelength(j);
                    difference = abs(band_color - led_index_color);
                    if (difference > best_difference) {
                        neopixel_data[j - 1] = 0x00FFFFFF;
                        restore_indices[i] = j - 1;
                        break;
                    }
                    best_difference = difference;
                }
                if (j == NUM_NEOPIXELS) {
                    neopixel_data[NUM_NEOPIXELS - 1] = 0x00FFFFFF;
                }
            }
            
            flash_neopixels();
            
            for (i = 0; i < NUM_BANDS; i++) {
                neopixel_data[restore_indices[i]] = led_index_to_rgb(restore_indices[i]);
            }
            
            /*
            for (i = 0; i < NUM_NEOPIXELS; i++) {
                led_wavelengths[i] = -1.0;
            }
            
            for (i = 0; i < NUM_BANDS; i++) {
                band_color = freq_band_to_wavelength(best_bands[i]);
                for (j = 0; j < NUM_NEOPIXELS; j++) {
                    led_index_color = led_index_to_wavelength(j);
                    neopixel_data[j] = (abs(band_color - led_index_color) < 75.0) ? wavelength_to_rgb(band_color, true) : wavelength_to_rgb(led_index_color, true);
                }
            }
            
            flash_neopixels();
            */
        }
    }
}
