# Accelerating PDE Approximations Through Parallel Architectures
Team member: Hao Wu
## Summary
I will compare and combine SIMD, CUDA, OpenMP and Open MPI libraries to efficiently approximate solutions to basic partial differential equations.
## Background
The specific types of PDEs I'm targeting are: 1D wave equations, and 2D Laplace, wave, and Poisson equations. 
The first 3 types are basic grid solvers: approximating 1D wave equations only involve updating a 1D array, whereas 2D Laplace and wave equations needs updating a 2D grid. Basic blocking and SIMD instructions can be used to parallelize updates. I'll compare memory and time costs of using CUDA threads, OpenMP tasks, and Open MPI tasks for allocating work. If I have extra time, I'll incorporate a dynamic scheduler to improve work balance.<br>
2D Poisson equations are much more complicated. They involve solving a large sparse linear system, which I intend to approximate by Gauss-Seidel, Jacobi, and SOR methods. Sequential solvers are already implemented, and I need to find ways to parallelize most parts of these methods. In particular, I will research ways on efficiently storing and multiplicating sparse matrices, probably by blocking. In this process, I'll construct a small, specialized, and efficient linear algebra library based on the optimal combination of different parallel architectures.
## Challenge
For the first 3 types of PDEs, it's not hard to parallelize: the difficulty lies in determining which scheme is best for which kind of updates. Comparing the performance on specific applications of these schemes provide me a deeper understanding of their differences.
For the last type of PDEs, solving large sparse linear systems typically cause work imbalance if work is not assigned carefully. So (probably dynamic) work assignment is the hardest part. Optimizing matrix operations for sparse matrices is also difficult.
## Resources
<ul><li>Machines: I'll first run on my personal computer with Intel 6-core i7 CPU and Nvidia Geforce GTX 1050 Ti Max-Q GPU. Then I'm going to optimize for Latedays Cluster machines with 2 6-core Xeon e5-2620 V3 CPUs and a NVIDIA K40 GPU + 60-core Xeon Phi 5110P.</li>
<li>Starter code: I implemented sequential version of G-S, Jacobi, SOR, as well as the whole 2D Poisson equations solver for a fixed simple set of boundary conditions.</li>
<li>Formulae: For 1D wave equations and 2D Laplace equations, I can refer to <a href="https://pdfs.semanticscholar.org/aa7a/561bc58ae4709ba45712e94971ebdb9e6330.pdf">Parallel Algorithms for Solving Partial Differential Equations</a>. For 2D Poisson equations, I can refer to <a href="http://elib.mi.sanu.ac.rs/files/journals/kjm/25/d001download.pdf">PARALLEL METHODS FOR SOLVING PARTIAL DIFFERENTIAL EQUATIONS</a>.</li></ul>

## Goals and Deliverables
### Plan to achieve: 
**First 3 equations:**<br>
<li>Comparison graphs of CUDA, OpenMP, and Open MPI performance in terms of runtime and memory cost, as well as arithmetic intensities for different problem sizes and machines.</li><br>


**2D Poisson equations:**<br>
<ul><li>A C++ library of matrix operations involved in solving sparse linear systems, implemented with SIMD instructions, and best-performing architecture possibly combining CUDA, OpenMP and Open MPI.</li>
<li>Comparison graphs of my utilization code versus the famous Eigen library in terms of runtime for dense and sparse matrices.</li>
<li>A 3D model based on the approximate solution of a 2D Poisson equation.</li></ul>

### Hope to achieve
<ul><li>Optimize matrix operations specifically for sparse matrices.</li>
<li>Dynamic work assignment.</li></ul>

## Platform Choice
I'm using C++ because I'm familiar with its SIMD, OpenMP and Open MPI instructions, and also because I wrote sequential version of 2D Poisson equation solver in C++. I'm targeting 2 types of machines in order to compare how different instruction sets behave in different runtime environments.

## Schedule (subject to frequent changes)


