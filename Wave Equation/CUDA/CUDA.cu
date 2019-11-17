#include "..\Sources\Background.h"

#include <cuda.h>
#include <cuda_runtime.h>

using namespace std;

__global__ void
init_kernel(float* f, float* g) {

    // compute overall index from position of thread in current block,
    // and given the block we are in
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    __m128 scalar = _mm_set_ps(XPERIOD, XPERIOD, XPERIOD, XPERIOD);
    __m128 init = _mm_set_ps(i + 3, i + 2, i + 1, i);
    __m128 scaled = _mm_mul_ps(init, scalar);
    __m128 sined = _mm_sin_ps(scaled);
    __m128 cosined = _mm_cos_ps(scaled);
    _mm_stream_ps(f + i, sined);
    _mm_stream_ps(g + i, cosined);
    result[index] = alpha * x[index] + y[index];
}

void init(float* f, float* g)
{
    __m128 scalar = _mm_set_ps(XPERIOD, XPERIOD, XPERIOD, XPERIOD);
    for (long long i = 0; i < DIST_STEPS; i += 4)
    {
        //f[i] = sin(i * XPERIOD);
        //g[i] = cos(i * XPERIOD);
        __m128 init = _mm_set_ps(i + 3, i + 2, i + 1, i);
        __m128 scaled = _mm_mul_ps(init, scalar);
        __m128 sined = _mm_sin_ps(scaled);
        __m128 cosined = _mm_cos_ps(scaled);
        _mm_stream_ps(f + i, sined);
        _mm_stream_ps(g + i, cosined);
    }
}

//modifies g to store second column
void secondU(const float* origU, float* g)
{
    __m128 scalar = _mm_set_ps(PERIOD, PERIOD, PERIOD, PERIOD);
    for (long long i = 0; i < DIST_STEPS / 4; i++)
    {
        __m128 streamU = _mm_load_ps(origU + 4 * i);
        __m128 streamG = _mm_load_ps(g + 4 * i);
        __m128 scaledG = _mm_mul_ps(streamG, scalar);
        __m128 sum = _mm_add_ps(streamU, scaledG);
        _mm_stream_ps(g + 4 * i, sum);
    }
}

//modifies thisU to store newest value
void iterate(float* thisU, float* nextU)
{
    __m128 fac1 = _mm_set_ps(FAC1, FAC1, FAC1, FAC1);
    __m128 fac2 = _mm_set_ps(FAC2, FAC2, FAC2, FAC2);
    
    // float *alwaysThisU = thisU;
    thisU[0] = FAC1 * nextU[0] + FAC2 * nextU[1] - thisU[0];
    thisU[1] = FAC1 * nextU[1] + FAC2 * (nextU[0] + nextU[2]) - thisU[1];
    thisU[2] = FAC1 * nextU[2] + FAC2 * (nextU[1] + nextU[3]) - thisU[2];
    thisU[3] = FAC1 * nextU[3] + FAC2 * (nextU[2] + nextU[4]) - thisU[3];

    float *prev = nextU + 3;
    float *curr = nextU + 4;
    float *next = nextU + 5;
    
    for (long long i = 4; i < DIST_STEPS - 4; i += 4)
    {
        long long j = i - 4;
        __m128 streamNextUNext = _mm_load_ps(next + j);
        __m128 streamNextUCurr = _mm_load_ps(curr + j);
        __m128 streamNextUPrev = _mm_load_ps(prev + j);
        __m128 streamThisU = _mm_load_ps(thisU + i);

        __m128 fac1Prod = _mm_mul_ps(fac1, streamNextUCurr);
        string empty = "";
        __m128 prevNextSum = _mm_add_ps(streamNextUPrev, streamNextUNext);
        __m128 fac2Prod = _mm_mul_ps(fac2, prevNextSum);
        __m128 nextUSum = _mm_add_ps(fac1Prod, fac2Prod);
        __m128 finalThisU = _mm_sub_ps(nextUSum, streamThisU);

        _mm_stream_ps(thisU + i, finalThisU);
        // thisU[i] = FAC1 * nextU[i] + FAC2 * ((i > 0 ? nextU[i - 1] : 0) + (i < DIST_STEPS - 1 ? nextU[i + 1] : 0)) - thisU[i];
    }

    long long endIndex = DIST_STEPS - 4;
    thisU[endIndex] = FAC1 * nextU[endIndex] + FAC2 * (nextU[endIndex - 1] + nextU[endIndex + 1]) - thisU[endIndex];
    thisU[endIndex + 1] = FAC1 * nextU[endIndex + 1] + FAC2 * (nextU[endIndex] + nextU[endIndex + 2]) - thisU[endIndex + 1];
    thisU[endIndex + 2] = FAC1 * nextU[endIndex + 2] + FAC2 * (nextU[endIndex + 1] + nextU[endIndex + 3]) - thisU[endIndex + 2];
    thisU[endIndex + 3] = FAC1 * nextU[endIndex + 3] + FAC2 * nextU[endIndex + 2] - thisU[endIndex + 3];
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

int main()
{
    float *thisU = (float *)malloc(DIST_STEPS * sizeof(float));
    float *nextU = (float *)malloc(DIST_STEPS * sizeof(float));
    double startTime =CycleTimer::currentSeconds();
    init(thisU, nextU);
    secondU(thisU, nextU);
    for (long long t = 0; t < MAX_T; t ++) {
        iterate(thisU, nextU);
        swap(thisU, nextU);
    }
    double endTime =CycleTimer::currentSeconds();
    cout << endTime - startTime << endl;
    //printArray(nextU);
    return 0;
}