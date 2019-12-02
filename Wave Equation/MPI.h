#include "Sources/Background.h"
#include "../vcpkg/packages/msmpi_x86-windows/include/mpi.h"

#define  MASTER		0

using namespace std;

class MPI{

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
	void test()
	{
		int  numtasks, taskid;

		MPI_Init(NULL, NULL);
		MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
		MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

		long length = DIST_STEPS / numtasks;
		if (length <= 4) exit(-1);

		float* thisU = (float*)malloc(length * sizeof(float));
		float* nextU = (float*)malloc(length * sizeof(float));

		//modifies thisU to store newest value
		//handles entire loop, use message id to stay syncs in loop
		MPI_Request req[2];
		MPI_Status stat[2];
		long finalElem = length - 1;

		double startTime = CycleTimer::currentSeconds();

		init(taskid, length, thisU, nextU);
		secondU(length, thisU, nextU);

		for (long t = 0; t < MAX_T; t++) {

			float tempPrev, tempNext;
			//start by sending all necessary info
			if (taskid != numtasks - 1)
			{
				MPI_Isend(nextU + finalElem, 1, MPI_FLOAT, taskid + 1, 2 * t, MPI_COMM_WORLD, req);
			}
			if (taskid != 0)
			{
				MPI_Isend(nextU, 1, MPI_FLOAT, taskid - 1, 2 * t + 1, MPI_COMM_WORLD, req + 1);
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
				MPI_Recv(&tempPrev, 1, MPI_FLOAT, taskid - 1, 2 * t, MPI_COMM_WORLD, &(stat[0]));
				thisU[0] += FAC1 * nextU[0] + FAC2 * (tempPrev + nextU[1]);
			}

			if (taskid == numtasks - 1)
			{
				thisU[finalElem] *= -1;
				thisU[finalElem] += FAC1 * nextU[finalElem] + FAC2 * nextU[finalElem - 1];
			}
			else
			{
				thisU[finalElem] *= -1;
				MPI_Recv(&tempNext, 1, MPI_FLOAT, taskid + 1, 2 * t + 1, MPI_COMM_WORLD, &(stat[1]));
				thisU[finalElem] += FAC1 * nextU[finalElem] + FAC2 * (nextU[finalElem - 1] + tempNext);
			}

			swap(thisU, nextU);
		}

		MPI_Finalize();

		double endTime = CycleTimer::currentSeconds();
		cout << "Total time: " << (endTime - startTime) << endl;

		free(thisU);
		free(nextU);
	}
};