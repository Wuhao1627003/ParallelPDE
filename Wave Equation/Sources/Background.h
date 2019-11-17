#include <cmath>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cstring>
#include "CycleTimer.h"
#include <smmintrin.h> // For _mm_stream_load_si128
#include <emmintrin.h> // For _mm_mul_ps
#include <immintrin.h>

#define C_CONST (1 << 4)
#define DIST_STEPS (long long)(1 << 5)
#define TIME_STEPS (long long)(1 << 10)
#define PERIOD (float)(1.0 / TIME_STEPS)
#define XPERIOD (float)(1.0 / DIST_STEPS)
#define S_CONST (C_CONST * PERIOD * DIST_STEPS)
#define FAC2 (S_CONST * S_CONST)
#define FAC1 (2 * (1 - FAC2))
#define MAX_T (long long)((1 << 3) * TIME_STEPS)

using namespace std;

void printTest(int lineNum)
{
    cout << "Reached line " << lineNum << endl;
}

void print128(__m128 floats)
{
    float *temp = (float *)calloc(4, sizeof(float));
    _mm_stream_ps(temp, floats);
    cout << "4 floats are: " << temp[0] << ", " << temp[1] << ", " << temp[2] << ", " << temp[3] << endl;
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