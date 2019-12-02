#include "MPI.h"
#include "OpenMP.h"
#include "Seq.h"
#include "SIMD.h"

int main()
{
	Seq seq;
	SIMD simd;
	MPI mpi;
	OpenMP openmp;
	seq.test();
	simd.test();
	mpi.test();
	openmp.test();
    return 0;
}