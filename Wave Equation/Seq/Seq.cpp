#include <cmath>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <malloc.h>

#define C_CONST (1 << 4)
#define DIST_STEPS (1 << 5)
#define TIME_STEPS (1 << 15)
#define PERIOD (1.0 / TIME_STEPS)
#define XPERIOD (1.0 / DIST_STEPS)
#define S_CONST (C_CONST * PERIOD * DIST_STEPS)
#define FAC2 (S_CONST * S_CONST)
#define FAC1 (2 * (1 - FAC2))
#define MAX_T ((1 << 5) * TIME_STEPS)

using namespace std;

void init(float* f, float* g)
{
    for (size_t i = 0; i < DIST_STEPS; i++)
    {
        f[i] = sin(i * XPERIOD);
        g[i] = cos(i * XPERIOD);
    }
}

//modifies g to store second column
void secondU(const float* origU, float* g)
{
    for (size_t i = 0; i < DIST_STEPS; i++)
    {
        g[i] = origU[i] + g[i] * PERIOD;
    }
}

//modifies thisU to store newest value
void iterate(float* thisU, const float* nextU)
{
    for (size_t i = 0; i < DIST_STEPS; i++)
    {
        thisU[i] = FAC1 * nextU[i] + FAC2 * ((i > 0 ? nextU[i - 1] : 0) + (i < DIST_STEPS - 1 ? nextU[i + 1] : 0)) - thisU[i];
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
    secondU(thisU, nextU);
    for (size_t t = 0; t < MAX_T; t ++) {
        iterate(thisU, nextU);
        swap(thisU, nextU);
    }
    printArray(nextU);
    return 0;
}