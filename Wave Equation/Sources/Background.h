#include <cmath>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include "CycleTimer.h"
#include <smmintrin.h> // For _mm_stream_load_si128
#include <emmintrin.h> // For _mm_mul_psk
#include <immintrin.h>

#define C_CONST (1 << 2)
#define DIST_STEPS (long)(1 << 10)
#define TIME_STEPS (long)(1 << 13)
#define PERIOD (float)(1.0 / TIME_STEPS)
#define XPERIOD (float)(1.0 / DIST_STEPS)
#define S_CONST (C_CONST * PERIOD * DIST_STEPS)
#define FAC2 (S_CONST * S_CONST)
#define FAC1 (2 * (1 - FAC2))
#define MAX_T (long)((1 << 5) * TIME_STEPS)