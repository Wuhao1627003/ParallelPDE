# Checkpoint
## What I've finished
I've already finished the sequential, SIMD, CUDA and MS-MPI versions of 1D wave equation, and have run them on a specified set of inputs.

## What I've changed from Proposal
Due to personal sickness and difficulty in installing environment, my progress has been significantly delayed, so I'm cutting the 2D Laplace equations portion, to focus more on 2D Poisson equations. Other changes include switching from Open MPI to MS-MPI because Windows doesn't support Open MPI, and possibly eliminating the usage of latedays cluster, because it can only run perl scripts.

## Why I think I can finish the rest
Setting up the environments was a huge bottleneck for me, and now that I've finished setting them up, the rest (coding) takes much shorter time to implement. Also, I'm planning to devote my whole Thanksgiving break to this project, so unless I become sick again, I think I can handle the rest on time.

## Deliverables
I haven't changed my plans for the deliverables mentioned in my proposal:

**1D Wave equations:**
<li>Comparison graphs of CUDA, OpenMP, and MS-MPI performance in terms of runtime and memory cost, as well as arithmetic intensities for different problem sizes.</li>
<br>

**2D Poisson equations:**
<li>A C++ library of matrix operations involved in solving sparse linear systems, implemented with SIMD instructions, and best-performing architecture possibly combining CUDA, OpenMP and MS-MPI.</li>
<li>Comparison graphs of my utilization code versus the famous Eigen library in terms of runtime for dense and sparse matrices.</li>
<li>A 3D model based on the approximate solution of a 2D Poisson equation.</li>

## Remaining concerns
If possible, I would still like to be able to run my code on latedays cluster, so my concerns are 1) how to run executables in latedays; 2) how to resolve cluster machines' complaint that SSE4.2 not supported for SIMD.