#include <cmath>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include "CycleTimer.h"
#include <smmintrin.h> // For _mm_stream_load_si128
#include <emmintrin.h> // For _mm_mul_psk
#include <immintrin.h>

#define C_CONST (0.0015)
#define DIST_STEPS (long)(1 << 15)
#define TIME_STEPS (long)(1 << 12)
#define PERIOD (float)(1.0 / TIME_STEPS)
#define XPERIOD (float)(1.0 / DIST_STEPS)
#define S_CONST (C_CONST * PERIOD * DIST_STEPS)
#define FAC2 (float)(S_CONST * S_CONST)
#define FAC1 (float)(2 * (1 - FAC2))
#define MAX_T (long)((1 << 3) * TIME_STEPS)

void printTest(int lineNum);
void initSeq(float* f, float* g);
void print256(__m256 floats);
void printArray(float* vec, long length);