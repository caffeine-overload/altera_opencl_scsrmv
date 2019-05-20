# altera_opencl_scsrmv
Sparse matrix dense vector multiplication benchmark for the Arria 10 FPGA

'main.cpp' Runs the benchmark, the arguments are positional and are as follows:

1 .  Path to opencl file, string (For altera, this file will have the extension aocx)

2 .  Path to benchmark file, string

3 .  Whether the file in the first argument is an opencl source or binary file. Single character, 's' or 'b'. 

4 .  OpenCL platform ID to use

5 .  OpenCL device ID to use

6 .  Number of induvidual benchmarks in the benchmark file. Integer, if in doubt just use 1.

7 .  OpenCL memory flag. Single character. 'd' is default, assumes device memory is separate to host. 'h' selects the 'CL_MEM_USE_HOST_PTR' flag. 'a' selects the 'CL_MEM_ALLOC_HOST_PTR' flag. For the integrated FPGA, use 'a'.

8 .  Datatype. String. 'float' or 'double'. The kernels in this repository are not optimised for double precision floats 
and the performance will be poor, so use 'float'. 

9 .  Matrix padding. Integer. The number of rows in the matrix must be a multiple of the block height specified 
when the kernel was compiled, this argument will add the specified number of padding rows to the matrix.

10 .  Reruns. Integer. The number of times to run the benchmark.


'app.py' can be used to automate running the benchmark for multiple benchmark files and opencl kernels.
