#include "Sources/Background.h"
#include "../vcpkg/packages/msmpi_x86-windows/include/mpi.h"

//#define MASTER 0
//#define BATCHSIZE (1 << 25)

using namespace std;

class MPISIMD {

private:
	__m256 fac1 = _mm256_set_ps(FAC1, FAC1, FAC1, FAC1, FAC1, FAC1, FAC1, FAC1);
	__m256 fac2 = _mm256_set_ps(FAC2, FAC2, FAC2, FAC2, FAC2, FAC2, FAC2, FAC2);
	
	__m256 scalarX = _mm256_set_ps(XPERIOD, XPERIOD, XPERIOD, XPERIOD, XPERIOD, XPERIOD, XPERIOD, XPERIOD);
	__m256 scalarP = _mm256_set_ps(PERIOD, PERIOD, PERIOD, PERIOD, PERIOD, PERIOD, PERIOD, PERIOD);

	void init(long taskid, long length, float* f, float* g)
	{
		long start = taskid * length;
		
		for (long index = 0; index < length; index += 8)
		{
			long i = index + start;
			__m256 init = _mm256_set_ps(i + 7, i + 6, i + 5, i + 4, i + 3, i + 2, i + 1, i);
			__m256 scaled = _mm256_mul_ps(init, scalarX);
			__m256 sined = _mm256_sin_ps(scaled);
			__m256 cosined = _mm256_cos_ps(scaled);
			_mm256_stream_ps(f + index, sined);
			_mm256_stream_ps(g + index, cosined);
		}
	}

	//modifies g to store second column
	void secondU(long length, const float* origU, float* g)
	{		
		for (long i = 0; i < length; i += 8)
		{
			__m256 streamU = _mm256_load_ps(origU + i);
			__m256 streamG = _mm256_load_ps(g + i);
			__m256 scaledG = _mm256_mul_ps(streamG, scalarP);
			__m256 sum = _mm256_add_ps(streamU, scaledG);
			_mm256_stream_ps(g + i, sum);
		}
	}

public:
	void test(int taskid, int numtasks)
	{

		long length = DIST_STEPS / numtasks;
		if (length <= 4) exit(-1);

		float* startU0 = (float*)malloc((DIST_STEPS + 4) * sizeof(float));
		float* startU1 = (float*)malloc((DIST_STEPS + 4) * sizeof(float));
		float* thisU = startU0 + (PtrToLong(startU0) % 32 == 0 ? 0 : 4);
		float* nextU = startU1 + (PtrToLong(startU1) % 32 == 0 ? 0 : 4);

		MPI_Request* req = (MPI_Request*)malloc(2 * BATCHSIZE * sizeof(MPI_Request));
		MPI_Status* stat = (MPI_Status*)malloc(2 * BATCHSIZE * sizeof(MPI_Status));
		float* temp = (float*)malloc(2 * BATCHSIZE * sizeof(float));
		long finalElem = length - 1;
		long endIndex = length - 8;

		double sumTime = 0;
		for (int n = 0; n < NUM_TESTS; n++) {
			double startTime = CycleTimer::currentSeconds();

			init(taskid, length, thisU, nextU);
			secondU(length, thisU, nextU);

			//modifies thisU to store newest value
			//handles entire loop, use message id to stay syncs in loop
			for (long t = 0; t < MAX_T; t++) {

				//start by sending all necessary info
				if (taskid != numtasks - 1)
				{
					MPI_Isend(nextU + finalElem, 1, MPI_FLOAT, taskid + 1, 2 * t, MPI_COMM_WORLD, req + (t % BATCHSIZE));
				}
				if (taskid != 0)
				{
					MPI_Isend(nextU, 1, MPI_FLOAT, taskid - 1, 2 * t + 1, MPI_COMM_WORLD, req + (t % BATCHSIZE) + BATCHSIZE);
				}

				//center block, not relying upon any communication
				for (int i = 1; i < 8; i++)
				{
					thisU[i] = FAC1 * nextU[i] + FAC2 * (nextU[i - 1] + nextU[i + 1]) - thisU[i];
				}

				for (long i = 8; i < length - 8; i += 8)
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

				//end by receiving all info, hiding latency by above computation
				if (taskid == 0)
				{
					thisU[0] *= -1;
					thisU[0] += FAC1 * nextU[0] + FAC2 * nextU[1];
				}
				else
				{
					thisU[0] *= -1;
					MPI_Recv(temp + (t % BATCHSIZE), 1, MPI_FLOAT, taskid - 1, 2 * t, MPI_COMM_WORLD, stat + (t % BATCHSIZE));
					thisU[0] += FAC1 * nextU[0] + FAC2 * (temp[t % BATCHSIZE] + nextU[1]);
				}

				if (taskid == numtasks - 1)
				{
					thisU[finalElem] *= -1;
					thisU[finalElem] += FAC1 * nextU[finalElem] + FAC2 * nextU[finalElem - 1];
				}
				else
				{
					thisU[finalElem] *= -1;
					MPI_Recv(temp + (t % BATCHSIZE) + BATCHSIZE, 1, MPI_FLOAT, taskid + 1, 2 * t + 1, MPI_COMM_WORLD, stat + (t % BATCHSIZE) + BATCHSIZE);
					thisU[finalElem] += FAC1 * nextU[finalElem] + FAC2 * (nextU[finalElem - 1] + temp[(t % BATCHSIZE) + BATCHSIZE]);
				}

				swap(thisU, nextU);
				if (t % BATCHSIZE == 0 && t != 0)
				{
					MPI_Waitall(2 * BATCHSIZE, req, stat);
				}
			}

			double endTime = CycleTimer::currentSeconds();
			sumTime += endTime - startTime;
		}
		
		if (taskid == 0)
		{
			printf("%f\n", sumTime / NUM_TESTS);
		}

		free(temp);
		free(req);
		free(stat);
		free(startU0);
		free(startU1);
	}
};