
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#define CL_HPP_ENABLE_EXCEPTIONS


#include <iostream>
#include <typeinfo>
#include <string>
#include <fstream>
#include <chrono>
#include <utility>
#include <string>
#include <vector>
#include <array>
#include <CL/cl.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <sstream>


#define err std::cerr
#define puts std::cout
#define br std::endl
#define ti(x) std::stoi(argv[x])

cl::Context context;
cl::CommandQueue queue;
const char* kernel_path;
int memory_operation;

typedef std::chrono::high_resolution_clock::time_point TimeVar;

#define duration(a) std::chrono::duration_cast<std::chrono::nanoseconds>(a).count()
#define timeNow() std::chrono::high_resolution_clock::now()

template<typename T> struct test_data{
    int num_rows;
    int num_cols;
    int nnz;
    T* values;
    int* cols;
    int* rptr;
    T a;
    T b;
    T* x;
    T* y;
    T* solution;
};

template<typename F, typename... Args>
double funcTime(F func, Args&&... args){
    TimeVar t1=timeNow();
    func(std::forward<Args>(args)...);
    return duration(timeNow()-t1);
}


cl::Program get_prog(const char* kernel_path, cl::Context context, bool src, std::vector<cl::Device> devicev){
    cl_int cl_status;
    std::ifstream file(kernel_path, std::ios::in | std::ios::binary | std::ios::ate);
    char *bins;
    int size;

    if (file.is_open()) {
        size = file.tellg();
        bins = new char[size];
        file.seekg(0, file.beg);
        file.read(bins, size);
        file.close();
    } else {
        std::cerr << "Source not found or failed to open [" << kernel_path << "]" << std::endl;
        exit(-1);
    }

    cl::Program program;
    if(src){
        cl::Program::Sources sources;

        std::pair<const char *, size_t> pair =
                std::make_pair((const char *) bins, size);
        sources.push_back(pair);

        program = cl::Program(context, sources);
    }else {

        cl::Program::Binaries binaries;

        std::pair<const char *, size_t> pair =
                std::make_pair((const char *) bins, size);
        binaries.push_back(pair);

        program = cl::Program(context, devicev, binaries);

    }

    cl_status = program.build(devicev);

    if (cl_status != CL_SUCCESS) {
        std::cerr << "Build fail: " << cl_status << std::endl;

        std::string name     = devicev[0].getInfo<CL_DEVICE_NAME>();
        std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devicev[0]);
        std::cerr << "Build log" << ":" << std::endl
                  << buildlog << std::endl;

        exit(-1);
    }

    return program;
}



/*
 * nnz rows cols  a  b
 * data[nnz]
 * col_indexes[nnz]
 * rowptrs[rows + 1]
 * x[cols]
 * y[rows]
 * solution[rows]
 */
template<typename T> void generate_data(const char* datafile_path, std::vector<struct test_data<T>> &matrixes, int reps, int padding){
    std::ifstream file(datafile_path, std::ios::in);

    for(int i = 0; i < reps; i++){
        std::string currentline;
        std::getline(file, currentline);
        std::stringstream tss(currentline);

        int nnz, rows, cols;
        T a;
        T b;

        //nnz << tss;
        //rows << tss;
        //cols << tss;
        //a << tss;
        //b << tss;
        tss >> nnz >> cols >> rows >> a >> b;

        //TODO Padding

#ifdef debug
puts << nnz << " " << rows << " " << cols << " " << a << " " << b << br;
#endif


        T* data = NULL;
        int* col_indexes = NULL;
        int* rowptrs = NULL;
        T* x = NULL;
        T* y = NULL;
        T* solution = NULL;

        std::array<int, 6> memerrors;
#define ALIGNMENT 1024

            memerrors[0] = posix_memalign((void **) &data, ALIGNMENT, (sizeof(T) * nnz) + padding);
            memerrors[1] = posix_memalign((void **) &col_indexes, ALIGNMENT, (sizeof(int) * nnz) + padding);
            memerrors[2] = posix_memalign((void **) &rowptrs, ALIGNMENT, (sizeof(int) * rows + 1) + padding + 1);
            memerrors[3] = posix_memalign((void **) &x, ALIGNMENT, (sizeof(T) * cols) + padding);
            memerrors[4] = posix_memalign((void **) &y, ALIGNMENT, (sizeof(T) * rows) + padding + 1);
            memerrors[5] = posix_memalign((void **) &solution, ALIGNMENT, (sizeof(T) * rows) + padding + 1);


        if(std::any_of(memerrors.begin(), memerrors.end(), [](int i){return i != 0;})){
            std::cerr << "Error allocaling aligned memory" << br;
            exit(-1);
        }

        std::getline(file, currentline);
        tss.clear();
        tss.str(currentline);
        for(int j = 0; j < nnz; j++){
            tss >> data[j];
#ifdef debug
            puts << data[j] << " ";
#endif
        }

#ifdef debug
        puts << br;
#endif

        std::getline(file, currentline);
        tss.clear();
        tss.str(currentline);
        for(int j = 0; j < nnz; j++){
            tss >> col_indexes[j];
#ifdef debug
            puts << col_indexes[j] << " ";
#endif
        }

#ifdef debug
        puts << br;
#endif

        std::getline(file, currentline);
        tss.clear();
        tss.str(currentline);
        for(int j = 0; j < rows + 1; j++){
            tss >> rowptrs[j];
#ifdef debug
            puts << rowptrs[j] << " ";
#endif
        }

#ifdef debug
        puts << br;
#endif

        std::getline(file, currentline);
        tss.clear();
        tss.str(currentline);
        for(int j = 0; j < cols; j++){
            tss >> x[j];
#ifdef debug
            puts << x[j] << " ";
#endif
        }

#ifdef debug
        puts << br;
#endif

        std::getline(file, currentline);
        tss.clear();
        tss.str(currentline);
        for(int j = 0; j < rows; j++){
            tss >> y[j];
#ifdef debug
            puts << y[j] << " ";
#endif
        }

#ifdef debug
        puts << br;
#endif

        std::getline(file, currentline);
        tss.clear();
        tss.str(currentline);
        for(int j = 0; j < rows; j++){
            tss >> solution[j];
#ifdef debug
            puts << solution[j] << " ";
#endif
        }

        if(rows % padding != 0){
            int new_row_count = ((rows / padding) + 1) * padding;
#ifdef debug
            puts << rows << " -> " << new_row_count << br;
#endif
           // int shortfall = new_row_count - rows;

            int* new_row_ptrs;
            T* new_y;
            T* new_solution;
            posix_memalign((void**) &new_row_ptrs, ALIGNMENT, sizeof(int) * new_row_count + 1);
            posix_memalign((void**) &new_y,        ALIGNMENT, sizeof(T)   * new_row_count);
            posix_memalign((void**) &new_solution, ALIGNMENT, sizeof(T)   * new_row_count);
            //Copy over initial values
            memcpy(new_row_ptrs, rowptrs, sizeof(int) * rows + 1);
            memcpy(new_y, y, sizeof(T)   * rows);
            memcpy(new_solution, solution, sizeof(T)   * rows);

            //Fill extra values
            new_row_ptrs[rows] = rowptrs[rows];//nnz;
            for(int xx = rows + 1; xx < new_row_count + 1; xx++){
                new_row_ptrs[xx] = rowptrs[rows];//nnz;
                new_y[xx - 1] = (T) 0;
                new_solution[xx - 1] = (T) 0;
            }

            free(rowptrs);
            free(y);
            free(solution);
            rowptrs = new_row_ptrs;
            y = new_y;
            solution = new_solution;
            rows = new_row_count;
/*
            rowptrs[rows] = nnz;


            for(int xx = rows + 1; xx < new_row_count + 1; xx++){
                rowptrs[xx] = nnz;
                y[xx - 1] = (T) 0;
                solution[xx - 1] = (T) 0;
            }
            rows = new_row_count;
            */
        }
#ifdef debug
        for(int xx = rows - 4; xx < rows + 1; xx++){
            puts << rowptrs[xx] << br;
        }
#endif

        struct test_data<T> td = {
                rows,
                cols,
                nnz,
                data,
                col_indexes,
                rowptrs,
                a,
                b,
                x,
                y,
                solution
        };

        matrixes.push_back(td);
    }
}

template <typename T> bool check(T* cpu_output, T* cl_output, int l){
    bool correct = true;
    if (typeid(T) == typeid(int)){
        correct = std::equal(cpu_output, cpu_output + l, cl_output);
    }else{
        for(int j = 0; j < l; j++){
#ifdef debug
            puts << "CL : " << cl_output[j] << " ANSWER : " << cpu_output[j] << br;
#endif
            if(fabs(cpu_output[j] - cl_output[j]) > fabs(cpu_output[j] * 0.01) + 0.0000001){
                correct = false;
                puts << "Wrong. Output " << cl_output[j] << " should be "
                << cpu_output[j] << "Difference " << fabs(cpu_output[j] - cl_output[j]) <<
                "Threshold " << cpu_output[j] * 0.001 << " at index " << j << br;
                break;
            }
        }
    }
    return correct;
}

template <typename T> void run_test(std::vector<struct test_data<T>> &matrices, cl::Kernel kern, const cl::NDRange glob, const cl::NDRange loc, const char* kernel_path, const char* datafile_path){
    bool noptr = memory_operation == 0 || memory_operation == 2;
    int memflags1 = memory_operation == 0 ? CL_MEM_READ_ONLY : (memory_operation == 1 ? CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR : CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR);
    int memflags2 = memory_operation == 0 ? CL_MEM_READ_WRITE : (memory_operation == 1 ? CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR : CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR);

    for(int i = 0; i < matrices.size(); i++) {
        struct test_data<T> test = matrices[i];
        cl_int status[5];
        cl::Buffer row_offset;
        cl::Buffer col;
        cl::Buffer val;
        cl::Buffer x;
        cl::Buffer y;

        row_offset = cl::Buffer(context, memflags1, sizeof(int) * (test.num_rows + 1), noptr ? NULL : test.rptr,
                                  &status[0]);
        col = cl::Buffer(context, memflags1, sizeof(int) * test.nnz, noptr ? NULL : test.cols, &status[1]);
        val = cl::Buffer(context, memflags1, sizeof(T) * test.nnz, noptr ? NULL : test.values, &status[2]);
        x = cl::Buffer(context, memflags1, sizeof(T) * test.num_cols, noptr ? NULL : test.x, &status[3]);
        y = cl::Buffer(context, memflags2, sizeof(T) * test.num_rows, noptr ? NULL : test.y, &status[4]);

        for (int j = 0; j < 5; j++) {
            if (status[j] != CL_SUCCESS) {
                std::cerr << "Opencl Create Buffer Error: " << status[j] << br;
                exit(-1);
            }
        }

        if(memory_operation == 2){
            //Not timing this since I assume an actual programmer would write to allocated memory to begin with
            void* ptr1 = queue.enqueueMapBuffer(row_offset, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(int) * (test.num_rows + 1));
            void* ptr2 = queue.enqueueMapBuffer(col       , CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(int) * (test.nnz));
            void* ptr3 = queue.enqueueMapBuffer(val       , CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(T) * (test.nnz));
            void* ptr4 = queue.enqueueMapBuffer(x         , CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(T) * (test.num_cols));
            void* ptr5 = queue.enqueueMapBuffer(y         , CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(T) * (test.num_rows));
            memcpy(ptr1, test.rptr, sizeof(int) * (test.num_rows + 1));
            memcpy(ptr2, test.cols, sizeof(int) * test.nnz);
            memcpy(ptr3, test.values, sizeof(T) * test.nnz);
            memcpy(ptr4, test.x,  sizeof(T) * test.num_cols);
            memcpy(ptr5, test.y, sizeof(T) * test.num_rows);
            queue.enqueueUnmapMemObject(row_offset, ptr1);
            queue.enqueueUnmapMemObject(col       , ptr2);
            queue.enqueueUnmapMemObject(val       , ptr3);
            queue.enqueueUnmapMemObject(x         , ptr4);
            queue.enqueueUnmapMemObject(y         , ptr5);
        }

        TimeVar t1=timeNow();
        if(memory_operation == 0){
            //Fill buffers
            queue.enqueueWriteBuffer(row_offset, CL_TRUE, 0, sizeof(int) * (test.num_rows + 1), test.rptr);
            queue.enqueueWriteBuffer(col, CL_TRUE, 0,  sizeof(int) * test.nnz, test.cols);
            queue.enqueueWriteBuffer(val, CL_TRUE, 0, sizeof(T) * test.nnz, test.values);
            queue.enqueueWriteBuffer(x, CL_TRUE, 0, sizeof(T) * test.num_cols, test.x);
            queue.enqueueWriteBuffer(y, CL_TRUE, 0, sizeof(T) * test.num_rows, test.y);
        }else if(memory_operation == 1){
            //Sync buffers
            void* ptr1 = queue.enqueueMapBuffer(row_offset, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(int) * (test.num_rows + 1));
            void* ptr2 = queue.enqueueMapBuffer(col       , CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(int) * (test.nnz));
            void* ptr3 = queue.enqueueMapBuffer(val       , CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(T) * (test.nnz));
            void* ptr4 = queue.enqueueMapBuffer(x         , CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(T) * (test.num_cols));
            void* ptr5 = queue.enqueueMapBuffer(y         , CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, sizeof(T) * (test.num_rows));

            queue.enqueueUnmapMemObject(row_offset, ptr1);
            queue.enqueueUnmapMemObject(col       , ptr2);
            queue.enqueueUnmapMemObject(val       , ptr3);
            queue.enqueueUnmapMemObject(x         , ptr4);
            queue.enqueueUnmapMemObject(y         , ptr5);
        }
            kern.setArg(0, test.num_rows);
            kern.setArg(1, test.a);
            kern.setArg(2, row_offset);
            kern.setArg(3, col);
            kern.setArg(4, val);
            kern.setArg(5, x);
            kern.setArg(6, test.b);
            kern.setArg(7, y);

        cl::Event evt;

        cl_int exe = queue.enqueueNDRangeKernel(kern, cl::NullRange, glob, loc, NULL, &evt);
        evt.wait();
        double elapsed = evt.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
                         evt.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        double total_elapsed = duration(timeNow()-t1);
        if(exe != CL_SUCCESS){
            std::cerr << "Kernel error " << exe << br;
        }
        bool correct;
        if(memory_operation == 0){
            T cl_output[test.num_rows] __attribute__ ((aligned (64)));
            queue.enqueueReadBuffer(y, CL_TRUE, 0, sizeof(T) * test.num_rows, cl_output);
            correct = check(test.solution, cl_output, test.num_rows);
        }else{
            void* ptr5 = queue.enqueueMapBuffer(y         , CL_TRUE, CL_MAP_READ, 0, sizeof(T) * (test.num_rows));
            correct = check(test.solution, (T*)ptr5, test.num_rows);
            queue.enqueueUnmapMemObject(y         , ptr5);
        }

        if(!correct){
            std::cout << "Not correct" << std::endl;
            exit(-1);
        }

        puts << kernel_path << "\t" << datafile_path << "\t" << test.num_rows << "\t" << test.num_cols << "\t" << test.nnz << "\t" << elapsed << "\t" << total_elapsed << br;
    }
}



/*
 * program path
 * datafile
 * src or bin
 * platform
 * dev
 * num_matrices_in_file
 * dev or host mem
 * datatype
 * padding
 * reruns
*/
int main (int argc, char* argv[]) {
    const char* kernel_path;
    const char* datafile_path;
    bool src;
    int reps;
    int mat_reps = 1;
    bool devmem;
    int platform_id, device_id;
    std::string datatype;
    int padding;

    kernel_path = argv[1];
    datafile_path = argv[2];
    src = argv[3][0] == 's';
    reps = ti(6);
    const cl::NDRange glob(1, 1, 1);
    const cl::NDRange loc(1, 1, 1);
    //devmem = argv[7][0] == 'd';
    if(argv[7][0] == 'd'){
        memory_operation = 0;
    }else if(argv[7][0] == 'h'){
        memory_operation = 1;
    }else if(argv[7][0] == 'a'){
        memory_operation = 2;
    }
    platform_id = ti(4);
    device_id = ti(5);
    datatype = std::string(argv[8]);
    padding = ti(9);
    if(argc > 10){
        mat_reps = ti(10);
    }

    cl_int cl_status;




    //Get platform and device
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    cl::Platform platform = platforms[platform_id];
    std::vector<cl::Device> devices;

    cl_status = platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    if (cl_status != CL_SUCCESS)
    {
        std::cout << "Problem with getting devices from platform"
                  << " [" << platform_id << "] " << platform.getInfo<CL_PLATFORM_NAME>()
                  << " error: [" << cl_status << "]" << std::endl;
    }
    cl::Device device = devices[device_id];
    std::vector<cl::Device> devicev;
    devicev.push_back(device);


    //Create command queue
    context = cl::Context(devices);


    queue = cl::CommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE);


    cl::Program program = get_prog(kernel_path, context, src, devicev);

    cl::Kernel kern(program, "spmdvm");


    if(datatype == "double"){
        std::vector<struct test_data<double>> matrixes;
        generate_data(datafile_path, matrixes, reps, padding);
        for(int i = 0; i < mat_reps; i++) run_test(matrixes, kern, glob, loc, kernel_path, datafile_path);
    }else if(datatype == "float"){
        std::vector<struct test_data<float>> matrixes;
        generate_data(datafile_path, matrixes, reps, padding);
        for(int i = 0; i < mat_reps; i++) run_test(matrixes, kern, glob, loc, kernel_path, datafile_path);
    }

}
