//*****************************************************************************
// Fast Fourier Transform Library
// Author: Zachary Zhou
//*****************************************************************************

#include "fft.h"

// Statically allocated array to hold output
static double complex output[NUM_SAMPLES];

//*****************************************************************************
// Reverses the bits of a number that can be encoded in at most 16 bits.
//*****************************************************************************
static uint16_t reverse(uint16_t x) {
    static uint16_t lut[NUM_SAMPLES];   // Lookup table
    uint16_t x_reversed = 0x0000;       // Holds the value to be returned
    uint16_t x_original = x;            // Stores the original value of 'x'
    uint8_t i;
    
    // If lookup table already contains the value, return it
    if ((lut[x] != 0) || (x == 0)) return lut[x];
    
    // Otherwise, compute the value and store it in the LUT
    for (i = 0; i < (log(NUM_SAMPLES) / log(2)); i++) {
        x_reversed <<= 1;
        x_reversed |= x & 0x0001;
        x >>= 1;
    }
    
    // Store the value in the LUT
    lut[x_original] = x_reversed;
    
    return x_reversed;
}

//*****************************************************************************
// Helper function for fft().
//*****************************************************************************
static void bit_reverse_copy(double complex *samples) {
    uint16_t i;
    for (i = 0; i < NUM_SAMPLES; i++) output[reverse(i)] = samples[i];
}

//*****************************************************************************
// Iterative implementation of the Cooley-Tukey radix-2 FFT algorithm.
//*****************************************************************************
double complex *fft(double complex *samples) {
    bit_reverse_copy(samples);
    
    for (uint16_t s = 1; s <= log2(NUM_SAMPLES); s++) {
        uint16_t m = pow(2, s);
        
        // Twiddle factor is a power of this
        double complex w_m = cexp(-2 * PI * I / m);
        
        for (uint16_t k = 0; k < NUM_SAMPLES; k += m) {
            // Twiddle factor
            double complex w = 1;
            
            for (uint16_t j = 0; j < (m / 2); j++) {
                // Butterfly operation
                double complex t = w * output[k + j + (m / 2)];
                double complex u = output[k + j];
                output[k + j] = u + t;
                output[k + j + (m / 2)] = u - t;
                
                // Adjust twiddle factor
                w *= w_m;
            }
        }
    }
    
    return output;
}
