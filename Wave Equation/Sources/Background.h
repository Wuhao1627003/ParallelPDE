#include <cmath>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cstring>
#include "CycleTimer.h"
#include <smmintrin.h> // For _mm_stream_load_si128
#include <emmintrin.h> // For _mm_mul_ps
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

using namespace std;

void printTest(int lineNum)
{
    cout << "Reached line " << lineNum << endl;
}

void print256(__m256 floats)
{
    float *temp = (float *)calloc(8, sizeof(float));
    _mm256_stream_ps(temp, floats);
    cout << "8 floats are: " << temp[0] << ", " << temp[1] << ", " << temp[2] << ", " << temp[3] << ", " << temp[4] << ", " << temp[5] << ", " << temp[6] << ", " << temp[7] << endl;
}

template<class T>
void printArray(T* vec)
{
    for (long long i = 0; i < DIST_STEPS; i ++)
    {
        cout << vec[i] << " ";
    }
    cout << endl;
}