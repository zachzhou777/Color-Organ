//*****************************************************************************
// Fast Fourier Transform Library
// Author: Zachary Zhou
//*****************************************************************************

#ifndef __FFT_H__
#define __FFT_H__

#include <complex.h>
#include <math.h>
#include <stdint.h>

#ifndef PI
#  define PI 3.14159265358979323846
#endif

#define NUM_SAMPLES 128  // Must be a power of 2

//*****************************************************************************
// Iterative implementation of the Cooley-Tukey radix-2 FFT algorithm.
//*****************************************************************************
double complex *fft(double complex *samples);

#endif
