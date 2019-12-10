#include "Sources/Background.h"
#include "../vcpkg/packages/msmpi_x86-windows/include/mpi.h"

#define MASTER 0
#define BATCHSIZE (1 << 25)

using namespace std;

class MPI {

private:
	void init(long taskid, long length, float* f, float* g)
	{
		long start = taskid * length;
		for (long index = 0; index < length; index++)
		{
			long i = index + start;
			f[index] = sin(i * XPERIOD);
			g[index] = cos(i * XPERIOD);
		}
	}

	//modifies g to store second column
	void secondU(long length, const float* origU, float* g)
	{
		for (long i = 0; i < length; i++)
		{
			g[i] = origU[i] + g[i] * PERIOD;
		}
	}

public:
	void test(int taskid, int numtasks)
	{
		long length = DIST_STEPS / numtasks;
		if (length <= 4) exit(-1);

		float* thisU = (float*)malloc(length * sizeof(float));
		float* nextU = (float*)malloc(length * sizeof(float));

		MPI_Request* req = (MPI_Request*)malloc(2 * BATCHSIZE * sizeof(MPI_Request));
		MPI_Status* stat = (MPI_Status*)malloc(2 * BATCHSIZE * sizeof(MPI_Status));
		float* temp = (float*)malloc(2 * BATCHSIZE * sizeof(float));
		long finalElem = length - 1;

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
				for (long i = 1; i < finalElem; i++)
				{
					thisU[i] *= -1;
					thisU[i] += FAC1 * nextU[i] + FAC2 * (nextU[i - 1] + nextU[i + 1]);
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
		free(thisU);
		free(nextU);
	}
};