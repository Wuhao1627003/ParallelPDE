#include "Sources/Background.h"

using namespace std;

class OpenMP{

private:
    void init(float* f, float* g)
    {
        #pragma omp parallel for
        for (long i = 0; i < DIST_STEPS; i++)
        {
            f[i] = sin(i * XPERIOD);
            g[i] = cos(i * XPERIOD);
        }
    }

    void secondU(const float* origU, float* g)
    {
        #pragma omp parallel for
        for (long i = 0; i < DIST_STEPS; i++)
        {
            g[i] = origU[i] + g[i] * PERIOD;
        }
    }

    void iterate(float *thisU, const float *nextU)
    {
        #pragma omp parallel for
        for (long i = 0; i < DIST_STEPS; i++)
        {
            thisU[i] = FAC1 * nextU[i] + FAC2 * ((i > 0 ? nextU[i - 1] : 0) + (i < DIST_STEPS - 1 ? nextU[i + 1] : 0)) - thisU[i];
            
        }
    }

public:
    void test()
    {
        float *thisU = (float *)malloc(DIST_STEPS * sizeof(float));
        float *nextU = (float *)malloc(DIST_STEPS * sizeof(float));
        double startInitTime = CycleTimer::currentSeconds();
        init(thisU, nextU);
        secondU(thisU, nextU);
        double endInitTime = CycleTimer::currentSeconds();
        cout << "Init time: " << (endInitTime - startInitTime) << endl;
        double startIterTime = CycleTimer::currentSeconds();
        for (long t = 0; t < MAX_T; t++)
        {
            iterate(thisU, nextU);
            swap(thisU, nextU);
        }
        double endIterTime = CycleTimer::currentSeconds();
        cout << "Total Iter time: " << (endIterTime - startIterTime) << endl;
        printArray(nextU, 32);
        free(thisU);
        free(nextU);
    }
};