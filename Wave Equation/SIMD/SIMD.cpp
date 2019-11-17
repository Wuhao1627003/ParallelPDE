#include <cmath>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <smmintrin.h> // For _mm_stream_load_si128
#include <emmintrin.h> // For _mm_mul_ps
#include <immintrin.h>
#include <cstring>

#define C_CONST (1 << 4)
#define DIST_STEPS (1 << 5)
#define TIME_STEPS (1 << 2)
#define PERIOD (1.0 / TIME_STEPS)
#define XPERIOD (1.0 / DIST_STEPS)
#define S_CONST (C_CONST * PERIOD * DIST_STEPS)
#define FAC2 (S_CONST * S_CONST)
#define FAC1 (2 * (1 - FAC2))
#define MAX_T ((1 << 2) * TIME_STEPS)

using namespace std;

void init(float* f, float* g)
{
    __m128 scalar = _mm_set_ps(XPERIOD, XPERIOD, XPERIOD, XPERIOD);
    for (size_t i = 0; i < DIST_STEPS / 4; i++)
    {
        //f[i] = sin(i * XPERIOD);
        //g[i] = cos(i * XPERIOD);
        __m128 init = _mm_set_ps(i, i + 1, i + 2, i + 3);
        __m128 scaled = _mm_mul_ps(init, scalar);
        __m128 sined = _mm_sin_ps(scaled);
        __m128 cosined = _mm_cos_ps(scaled);
        _mm_stream_ps(f + 4 * i, sined);
        _mm_stream_ps(g + 4 * i, cosined);
    }
}

//modifies g to store second column
void secondU(const float* origU, float* g)
{
    __m128 scalar = _mm_set_ps(PERIOD, PERIOD, PERIOD, PERIOD);
    for (size_t i = 0; i < DIST_STEPS / 4; i++)
    {
        __m128 streamU = _mm_load_ps(origU + 4 * i);
        __m128 streamG = _mm_load_ps(g + 4 * i);
        __m128 scaledG = _mm_mul_ps(streamG, scalar);
        __m128 sum = _mm_add_ps(streamU, scaledG);
        _mm_stream_ps(g + 4 * i, sum);
    }
}

void printTest(int lineNum)
{
    cout << "Reached line " << lineNum << endl;
}

//modifies thisU to store newest value
void iterate(float* thisU, float* nextU)
{
    __m128 fac1 = _mm_set_ps(FAC1, FAC1, FAC1, FAC1);
    __m128 fac2 = _mm_set_ps(FAC2, FAC2, FAC2, FAC2);
    float *prev = nextU;
    float *next = nextU + 2;
    float *alwaysThisU = thisU;
    thisU[0] = FAC1 * nextU[0] + FAC2 * nextU[1] - thisU[0];
    
    for (size_t i = 1; i < DIST_STEPS - 4; i += 4)
    {
        __m128 streamNextUNext, streamThisU, streamNextU;
        if (i == DIST_STEPS - 3)
        {
            next = (float *)calloc(4, sizeof(float));
            memcpy(next, nextU + i, 2 * sizeof(float));
            streamNextUNext = _mm_load_ps(next);

            nextU = (float *)calloc(4, sizeof(float));
            memcpy(nextU, prev + i, 3 * sizeof(float));
            streamNextU = _mm_load_ps(nextU);

            thisU = (float *)calloc(4, sizeof(float));
            memcpy(thisU, alwaysThisU + i, 3 * sizeof(float));
            streamThisU = _mm_load_ps(thisU);
        }
        else
        {
            streamNextUNext = _mm_load_ps(next + (i - 1));
            // float* tmp = (float *)calloc(5, sizeof(float));
            // _mm_stream_ps(tmp + 1, streamNextUNext);
            // cout << tmp[0] << tmp[1] << tmp[2] << tmp[3] << tmp[4] << endl;
            printf("%p \n", next + (i - 1));
            streamNextU = _mm_load_ps(nextU + i);
            streamThisU = _mm_load_ps(thisU + i);
        }
        
        __m128 streamNextUPrev = _mm_load_ps(prev + (i - 1));
        __m128 fac1Prod = _mm_mul_ps(fac1, streamNextU);
        __m128 prevNextSum = _mm_add_ps(streamNextUPrev, streamNextUNext);
        __m128 fac2Prod = _mm_mul_ps(fac2, prevNextSum);
        __m128 nextUSum = _mm_add_ps(fac1Prod, fac2Prod);
        __m128 finalThisU = _mm_sub_ps(nextUSum, streamThisU);
        printTest(__LINE__);
        _mm_stream_ps(thisU + i, finalThisU);
        printTest(__LINE__);
        if (i == DIST_STEPS - 3)
        {
            free(next);
            free(nextU);
            memcpy(alwaysThisU + i, thisU, 3 * sizeof(float));
            free(thisU);
        }
        // thisU[i] = FAC1 * nextU[i] + FAC2 * ((i > 0 ? nextU[i - 1] : 0) + (i < DIST_STEPS - 1 ? nextU[i + 1] : 0)) - thisU[i];
    }
}

template<class T>
void printArray(T* vec)
{
    for (size_t i = 0; i < DIST_STEPS; i ++)
    {
        cout << vec[i] << " ";
    }
    cout << endl;
}

int main()
{
    float *thisU = (float *)malloc(DIST_STEPS * sizeof(float));
    float *nextU = (float *)malloc(DIST_STEPS * sizeof(float));
    init(thisU, nextU);
    printArray(nextU);
    secondU(thisU, nextU);
    printArray(nextU);
    for (size_t t = 0; t < MAX_T; t ++) {
        iterate(thisU, nextU);
        printArray(thisU);
        swap(thisU, nextU);
    }
    printArray(nextU);
    return 0;
}