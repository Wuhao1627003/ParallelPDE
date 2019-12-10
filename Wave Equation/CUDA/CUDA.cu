#include "..\Sources\Background.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>

#define THREADSPERBLOCK (1 << 12) 
#define NUMBLOCKS (DIST_STEPS / THREADSPERBLOCK)

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
    __shared__ float currBlock[THREADSPERBLOCK + 2];

    currBlock[threadIdx.x + 1] = nextU[blockIdx.x * blockDim.x + threadIdx.x];

    if (threadIdx.x == 0)
    {
        if (blockIdx.x != 0)
        {
            currBlock[0] = nextU[blockIdx.x * blockDim.x - 1];
        }
        else
        {
            currBlock[0] = 0;
        }
        if (blockIdx.x != NUMBLOCKS - 1)
        {
            currBlock[blockDim.x + 1] = nextU[(blockIdx.x + 1) * blockDim.x];
        }
        else
        {
            currBlock[blockDim.x + 1] = 0;
        }
    }
    __syncthreads();

    long index = blockIdx.x * blockDim.x + threadIdx.x;
    long i = threadIdx.x;
    thisU[index] = FAC1 * currBlock[i + 1] + FAC2 * (currBlock[i] + currBlock[i + 2]) - thisU[index];
}

void cudaWrap(float *startU0, float *startU1)
{
    float *prev, *next;
    cudaMalloc(&prev, (size_t)(DIST_STEPS) * sizeof(float));
    cudaMalloc(&next, (size_t)(DIST_STEPS) * sizeof(float));

    cudaMemcpy(prev, startU0, (size_t)DIST_STEPS * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(next, startU1, (size_t)DIST_STEPS * sizeof(float), cudaMemcpyHostToDevice);

    double startTime = CycleTimer::currentSeconds();

    init_kernel<<<NUMBLOCKS, THREADSPERBLOCK>>>(prev, next);
    cudaThreadSynchronize();
    secondU_kernel<<<NUMBLOCKS, THREADSPERBLOCK>>>(prev, next);
    cudaThreadSynchronize();

    for (long t = 0; t < MAX_T; t++)
    {
        iterate_kernel<<<NUMBLOCKS, THREADSPERBLOCK, (THREADSPERBLOCK + 2) * sizeof(float)>>>(prev, next);
        cudaThreadSynchronize();
        swap(prev, next);
    }
    
    double endTime = CycleTimer::currentSeconds();
    printf("%f\n", endTime - startTime);
    cudaMemcpy(startU1, next, (size_t)DIST_STEPS * sizeof(float), cudaMemcpyDeviceToHost);
    //printArray(startU1, DIST_STEPS);
    cudaFree(prev);
    cudaFree(next);
}

int main()
{
    float *thisU = (float *)calloc(DIST_STEPS, sizeof(float));
    float *nextU = (float *)calloc(DIST_STEPS, sizeof(float));

    for (int i = 0; i < NUM_TESTS; i ++)
        cudaWrap(thisU, nextU);
    
    free(thisU);
    free(nextU);
    return 0;
}