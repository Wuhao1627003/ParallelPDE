#include "..\Sources\Background.h"

#include <cuda.h>
#include <cuda_runtime.h>

#define THREADSPERBLOCK (1 << 2) 
#define NUMBLOCKS (DIST_STEPS / THREADSPERBLOCK)

using namespace std;

__global__ void
init_kernel(float* f, float* g) 
{
    // compute overall index from position of thread in current block,
    // and given the block we are in
    long index = blockIdx.x * blockDim.x + threadIdx.x;
    f[index] = sin(index * XPERIOD);
    g[index] = cos(index * XPERIOD);
}

//modifies g to store second column
__global__ void
secondU_kernel(const float* origU, float* g) 
{
    long index = blockIdx.x * blockDim.x + threadIdx.x;
    g[index] = origU[index] + g[index] * PERIOD;
}

//modifies thisU to store newest value
__global__ void
iterate_kernel(float* thisU, const float* nextU)
{
    long index = blockIdx.x * blockDim.x + threadIdx.x;
    thisU[index] = FAC1 * nextU[index] + FAC2 * ((index > 0 ? nextU[index - 1] : 0) + (index < DIST_STEPS - 1 ? nextU[index + 1] : 0)) - thisU[index];
}

void cudaWrap(float *startU0, float *startU1)
{
    float *prev, *next;
    cudaMalloc(&prev, (size_t)(DIST_STEPS) * sizeof(float));
    cudaMalloc(&next, (size_t)(DIST_STEPS) * sizeof(float));

    cudaMemcpy(prev, startU0, (size_t)DIST_STEPS * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(next, startU1, (size_t)DIST_STEPS * sizeof(float), cudaMemcpyHostToDevice);

    double startInitTime = CycleTimer::currentSeconds();

    init_kernel<<<NUMBLOCKS, THREADSPERBLOCK>>>(prev, next);
    cudaThreadSynchronize();
    secondU_kernel<<<NUMBLOCKS, THREADSPERBLOCK>>>(prev, next);
    cudaThreadSynchronize();

    double endInitTime = CycleTimer::currentSeconds();
    cout << "Init time: " << (endInitTime - startInitTime) << endl;
    double startIterTime = CycleTimer::currentSeconds();
    double startPartIterTime = startIterTime;
    for (long t = 0; t < MAX_T; t++)
    {
        iterate_kernel<<<NUMBLOCKS, THREADSPERBLOCK>>>(prev, next);
        cudaThreadSynchronize();
        swap(prev, next);
        if (t % (MAX_T >> 3) == 0 && t != 0)
        {
            double endPartIterTime = CycleTimer::currentSeconds();
            cout << "Part Iter time: " << (endPartIterTime - startPartIterTime) << endl;
            startPartIterTime = CycleTimer::currentSeconds();
        }
    }
    
    double endIterTime = CycleTimer::currentSeconds();
    cout << "Total Iter time: " << (endIterTime - startIterTime) << endl;
    cudaMemcpy(startU1, next, (size_t)DIST_STEPS * sizeof(float), cudaMemcpyDeviceToHost);
    //printArray(startU1, DIST_STEPS);
    cudaFree(prev);
    cudaFree(next);

}

int main()
{
    float *thisU = (float *)calloc(DIST_STEPS, sizeof(float));
    float *nextU = (float *)calloc(DIST_STEPS, sizeof(float));

    cudaWrap(thisU, nextU);
    
    free(thisU);
    free(nextU);
    return 0;
}