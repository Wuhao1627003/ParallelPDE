#include "Sources/Background.h"
#include <omp.h>

using namespace std;

class OpenMPSIMD {

private:
    __m256 fac1 = _mm256_set_ps(FAC1, FAC1, FAC1, FAC1, FAC1, FAC1, FAC1, FAC1);
    __m256 fac2 = _mm256_set_ps(FAC2, FAC2, FAC2, FAC2, FAC2, FAC2, FAC2, FAC2);
    long endIndex = DIST_STEPS - 8;

    void init(float* f, float* g)
    {
        long i;
        __m256 scalar = _mm256_set_ps(XPERIOD, XPERIOD, XPERIOD, XPERIOD, XPERIOD, XPERIOD, XPERIOD, XPERIOD);
#pragma omp parallel for
        for (i = 0; i < DIST_STEPS; i += 8)
        {
            __m256 init = _mm256_set_ps(i + 7, i + 6, i + 5, i + 4, i + 3, i + 2, i + 1, i);
            __m256 scaled = _mm256_mul_ps(init, scalar);
            __m256 sined = _mm256_sin_ps(scaled);
            __m256 cosined = _mm256_cos_ps(scaled);
            _mm256_stream_ps(f + i, sined);
            _mm256_stream_ps(g + i, cosined);
        }
    }

    void secondU(const float* origU, float* g)
    {
        long i;
        __m256 scalar = _mm256_set_ps(PERIOD, PERIOD, PERIOD, PERIOD, PERIOD, PERIOD, PERIOD, PERIOD);
#pragma omp parallel for
        for (long i = 0; i < DIST_STEPS; i += 8)
        {
            __m256 streamU = _mm256_load_ps(origU + i);
            __m256 streamG = _mm256_load_ps(g + i);
            __m256 scaledG = _mm256_mul_ps(streamG, scalar);
            __m256 sum = _mm256_add_ps(streamU, scaledG);
            _mm256_stream_ps(g + i, sum);
        }
    }

    void iterate(float* thisU, const float* nextU)
    {
        thisU[0] = FAC1 * nextU[0] + FAC2 * nextU[1] - thisU[0];
        for (int i = 1; i < 8; i++)
        {
            thisU[i] = FAC1 * nextU[i] + FAC2 * (nextU[i - 1] + nextU[i + 1]) - thisU[i];
        }

#pragma omp parallel for
        for (long i = 8; i < DIST_STEPS - 8; i += 8)
        {
            __m256 streamNextUNext = _mm256_load_ps(nextU + i + 1);
            __m256 streamNextUCurr = _mm256_load_ps(nextU + i);
            __m256 streamNextUPrev = _mm256_load_ps(nextU + i - 1);
            __m256 streamThisU = _mm256_load_ps(thisU + i);

            __m256 fac1Prod = _mm256_mul_ps(fac1, streamNextUCurr);
            short empty;
            __m256 prevNextSum = _mm256_add_ps(streamNextUPrev, streamNextUNext);
            __m256 fac2Prod = _mm256_mul_ps(fac2, prevNextSum);
            __m256 nextUSum = _mm256_add_ps(fac1Prod, fac2Prod);
            __m256 finalThisU = _mm256_sub_ps(nextUSum, streamThisU);

            _mm256_stream_ps(thisU + i, finalThisU);
        }

        for (int i = 0; i < 7; i++)
        {
            thisU[endIndex + i] = FAC1 * nextU[endIndex + i] + FAC2 * (nextU[endIndex + i - 1] + nextU[endIndex + i + 1]) - thisU[endIndex + i];
        }
        thisU[endIndex + 7] = FAC1 * nextU[endIndex + 7] + FAC2 * nextU[endIndex + 6] - thisU[endIndex + 7];
    }

public:
    void test()
    {
        float* thisU = (float*)malloc(DIST_STEPS * sizeof(float));
        float* nextU = (float*)malloc(DIST_STEPS * sizeof(float));

        double sumTime = 0;
        for (int n = 0; n < NUM_TESTS; n++) {
            double startTime = CycleTimer::currentSeconds();

            init(thisU, nextU);
            secondU(thisU, nextU);

            for (long t = 0; t < MAX_T; t++)
            {
                iterate(thisU, nextU);
                swap(thisU, nextU);
            }

            double endTime = CycleTimer::currentSeconds();
            sumTime += endTime - startTime;
        }
        printf("OpenMPSIMD Total time: %f\n", sumTime / NUM_TESTS);

        //printArray(DIST_STEPS, 32);

        free(thisU);
        free(nextU);
    }
};