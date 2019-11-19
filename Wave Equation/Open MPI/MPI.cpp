#include "..\Sources\Background.h"
#include <mpi.h>
#define  MASTER		0


using namespace std;

void initMPI(long taskid, long length, float* f, float* g)
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
void secondUMPI(long length, const float* origU, float* g)
{
	for (long i = 0; i < length; i++)
	{
		g[i] = origU[i] + g[i] * PERIOD;
	}
}

//modifies thisU to store newest value
//handles entire loop, use message id to stay syncs in loop
void iterateMPI(long taskid, long length, float* thisU, const float* nextU)
{
	if (taskid == 0)
	{
		for (long i = 0; i < DIST_STEPS; i++)
		{
			thisU[i] = FAC1 * nextU[i] + FAC2 * ((i > 0 ? nextU[i - 1] : 0) + (i < DIST_STEPS - 1 ? nextU[i + 1] : 0)) - thisU[i];

		}
	}
	for (long i = 0; i < DIST_STEPS; i++)
	{
		thisU[i] = FAC1 * nextU[i] + FAC2 * ((i > 0 ? nextU[i - 1] : 0) + (i < DIST_STEPS - 1 ? nextU[i + 1] : 0)) - thisU[i];

	}
}

int main()
{
	int  numtasks, taskid;
	MPI_Status status;

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);




	MPI_Finalize();
	return 0;
}
