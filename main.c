//*****************************************************************************
// Color Organ
// Usage: 
// Author: Zachary Zhou
//*****************************************************************************

#include "main.h"

double complex left_channel_samples[NUM_SAMPLES];
double complex right_channel_samples[NUM_SAMPLES];
double normalized_output[NUM_SAMPLES / 2];

// Interrupt flag
volatile bool TIMER0A_FLAG;

void get_samples(void) {
	uint16_t i;
	for (i = 0; i < NUM_SAMPLES; i++) {
		audio_get_data();
		left_channel_samples[i] = (double complex) audio_left_channel() / 0xFFF;
		right_channel_samples[i] = (double complex) audio_right_channel() / 0xFFF;
	}
}

__inline void disable_interrupts(void) {
	__asm{CPSID i}
}

__inline void enable_interrupts(void) {
	__asm{CPSIE i}
}

void timer_config(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, 5000);
	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER0_BASE, TIMER_A);
}

//*****************************************************************************
// Initialize and configure all the relevant hardware.
//*****************************************************************************
void initialize_hardware(void) {
	disable_interrupts();
	
	timer_config();	
	audio_config();
	neopixels_config();
	print_config();
	
  enable_interrupts();
}

//*****************************************************************************
// Timer0A ISR.
//*****************************************************************************
void TIMER0A_Handler(void) {
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TIMER0A_FLAG = true;
}

//*****************************************************************************
// Tests functions.
//*****************************************************************************
int main(void) {
	initialize_hardware();
	//*
	Color red = {0x00, 0x00, 0x00};
	Color green = {0x00, 0xFF, 0x00};
	uint16_t sample_num = 0;
	uint16_t i;
	double max_magnitude;
	uint16_t max_index;
	double complex *raw_output;
	
	while (1) {
		if (TIMER0A_FLAG) {
			TIMER0A_FLAG = false;
			audio_get_data();
			left_channel_samples[sample_num] = (double complex) audio_left_channel() / 0xFFF;
			right_channel_samples[sample_num] = (double complex) audio_right_channel() / 0xFFF;
			sample_num++;
			if (sample_num >= NUM_SAMPLES) {
				sample_num = 0;
				raw_output = fft(left_channel_samples);
				/*printf("*** START OF TRANSMISSION ***\n");
				for (i = 0; i < NUM_SAMPLES; i++) {
					printf("%d: %f\n", i, cabs(raw_output[i]));
				}
				printf("*** END OF TRANSMISSION ***\n");*/
				printf("*** START OF TRANSMISSION ***\n");
				for (i = 0; i < (NUM_SAMPLES / 2); i++) {
					normalized_output[i] = log2(cabs(raw_output[i]));
					printf("%d: %f, %f\n", i, cabs(raw_output[i]), normalized_output[i]);
				}
				printf("*** END OF TRANSMISSION ***\n");
				max_magnitude = normalized_output[1];
				max_index = 1;
				for (i = 2; i < (NUM_SAMPLES / 2); i++) {
					if (normalized_output[i] > max_magnitude) {
						max_magnitude = normalized_output[i];
						max_index = i;
					}
				}
				for (i = 0; i < NUM_NEOPIXELS; i++) {
					neopixels_set_LED_wavelength(i, 780 - ((2.0 * max_index / NUM_SAMPLES) * 400));
				}
				neopixels_send_data();
			}
		}
	}
	//*/
	
	/*
	Color red = {0xFF, 0x00, 0x00};
	Color green = {0x00, 0xFF, 0x00};
	double complex *output;
	uint16_t i;
	double max_magnitude;
	uint16_t max_index;
	while (1) {
		get_samples();
		output = fft(left_channel_samples);
		//output[4] = 10000;	// Debug statement delet dis
		max_magnitude = cabs(output[0]);
		max_index = 0;
		for (i = 1; i < NUM_SAMPLES; i++) {
			if (cabs(output[i]) > max_magnitude) {
				max_magnitude = cabs(output[i]);
				max_index = i;
			}
		}
		for (i = 0; i < NEOPIXELS_NUM_LEDS; i++) neopixels_set_LED_color(i, &red);
		neopixels_set_LED_color(max_index, &green);
		neopixels_send_data();
	}
	//*/
	/*
	Color red = {0xFF, 0x00, 0x00};
	Color green = {0x00, 0xFF, 0x00};
	Color blue = {0x00, 0x00, 0xFF};
	Color colorless = {0x00, 0x00, 0x00};
	
	uint32_t i, audio;
	double audiod;
	Color color;
	
	while (1) {
		audio_get_data();
		audio = audio_left_channel();
		audiod = fabs((2.0 * audio / 0xFFF) - 1);
		for (i = 0; i < NEOPIXELS_NUM_LEDS; i++) {
			audio = audiod * 0xFFF;
			color.red = (0xF00 & audio) >> 8;
			color.green = (0xF0 & audio) >> 4;
			color.blue = 0xF & audio;
			
			if (audiod < 0.25) color = colorless;
			else if (audiod < 0.5) color = red;
			else if (audiod < 0.75) color = green;
			else color = blue;
			
			neopixels_set_LED_color(i, &color);
		}
		neopixels_send_data();
		for (i = 0; i < 50000; i++) {
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
	}
	//*/
	/*
	uint32_t i, audio;
	Color color;
	
	while (1) {
		audio_get_data();
		audio = audio_right_channel();
		color.red = (0xF00 & audio) >> 8;
		color.green = (0xF0 & audio) >> 4;
		color.blue = 0xF & audio;
		for (i = 0; i < NEOPIXELS_NUM_LEDS; i++) neopixels_set_LED_color(i, &color);
		neopixels_send_data();
		for (i = 0; i < 50000; i++) {
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
	}
	//*/
	/*
	Color red = {0xFF, 0x00, 0x00};
	Color green = {0x00, 0xFF, 0x00};
	Color blue = {0x00, 0x00, 0xFF};
	Color colorless = {0x00, 0x00, 0x00};
	Color color;
	uint32_t i, audio1, audio2;
	audio_get_data();
	audio1 = audio_right_channel();
	while (1) {
		audio_get_data();
		audio2 = audio_right_channel();
		if (abs(audio1 - audio2) < 0.25 * 0xFFF) color = colorless;
		else if (abs(audio1 - audio2) < 0.5 * 0xFFF) color = red;
		else if (abs(audio1 - audio2) < 0.75 * 0xFFF) color = green;
		else color = blue;
		for (i = 0; i < NEOPIXELS_NUM_LEDS; i++) neopixels_set_LED_color(i, &color);
		neopixels_send_data();
		audio1 = audio2;
		for (i = 0; i < 50000; i++) {
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
	}
	//*/
	
	/*
	Color red = {0xFF, 0x00, 0x00};
	Color green = {0x00, 0xFF, 0x00};
	Color blue = {0x00, 0x00, 0xFF};
	Color colorless = {0x00, 0x00, 0x00};
	Color *color;
	uint32_t i, j, k;
	
	audio_get_data();
	uint32_t bound = (uint32_t) ((double) audio_right_channel() * NEOPIXELS_NUM_LEDS / 0xFFF);
	for (i = 0; i < bound; i++) {
		neopixels_set_LED_color(i, &green);
	}
	for (i = bound; i < NEOPIXELS_NUM_LEDS; i++) {
		neopixels_set_LED_color(i, &red);
	}
	neopixels_send_data();
	//*/
	
	/*
	for (i = 0; i < NEOPIXELS_NUM_LEDS; i++) {
		switch (i % 3) {
			case 0:
				neopixels_set_LED_color(i, &red);
				break;
			case 1:
				neopixels_set_LED_color(i, &green);
				break;
			default:
				neopixels_set_LED_color(i, &blue);
		}
	}
	neopixels_send_data();
	//*/
	
	/*
	Color red = {0xFF, 0x00, 0x00};
	Color green = {0x00, 0xFF, 0x00};
	Color blue = {0x00, 0x00, 0xFF};
	Color colorless = {0x00, 0x00, 0x00};
	Color *color;
	uint32_t i, j, k;
	for (i = 0; i < NEOPIXELS_NUM_LEDS; i++) neopixels_set_LED_color(i, &colorless);
	
	while (1) {
		for (i = 0; i < 3; i++) {
			switch (i % 3) {
				case 0:
					color = &red;
					break;
				case 1:
					color = &green;
					break;
				default:
					color = &blue;
			}
			for (j = 0; j < NEOPIXELS_NUM_LEDS; j++) {
				for (k = 0; k < NEOPIXELS_NUM_LEDS; k++) {
					if (k <= j) neopixels_set_LED_color(k, color);
					else neopixels_set_LED_color(k, &colorless);
				}
				neopixels_send_data();
				for (k = 0; k < 25000; k++) __asm {
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
		}
	}
	//*/
}
