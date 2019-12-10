//#include <omp.h>
//#include "OpenMP.h"
//#include "Seq.h"
//#include "OpenMPSIMD.h"
//#include "SIMD.h"
#include "MPI.h"
#include "MPISIMD.h"

using namespace std;

int main()
{
#if ICL
	Seq seq;
	OpenMP openmp;
	OpenMPSIMD openmpsimd;
	SIMD simd;

	printf("Time steps: %ld Dist steps: %ld\n", MAX_T, DIST_STEPS);

	seq.test();

	simd.test();

	omp_set_dynamic(0);
	int threadNums[] = {2, 4, 8};
	for (int i : threadNums)
	{
		omp_set_num_threads(i);
		openmp.test();
	}
	

	//openmpsimd.test();
#else
	MPI mpi;
	MPISIMD mpisimd;
	int numtasks, taskid;

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	//if (taskid == 0) printf("Time steps: %ld Dist steps: %ld\n", MAX_T, DIST_STEPS);

	mpi.test(taskid, numtasks);
	
	mpisimd.test(taskid, numtasks);

	MPI_Finalize();
#endif
    return 0;
}