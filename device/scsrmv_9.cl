#ifndef block_height
#define block_height 2
#endif
#define FADD_LATENCY 5
#ifndef DATAUNROLL
#error "please define dataunroll"
#endif


#if defined(DOUBLE)
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#define float double
#define ZERO 0.0
#define ugentype long
#else
#define ZERO 0.0f
#define ugentype int
#endif

__kernel
__attribute__((reqd_work_group_size(1,1,1)))
void spmdvm (
        const int num_rows,
        const float alpha,
        __global const int * const restrict row_offset,
        __global const int * const restrict col,
        __global const float * const restrict val,
        __global
#ifdef VOLATILEX
volatile
#else
const
#endif
float * const restrict x,
        const float beta,
        __global float * restrict y)
{

    for(int block_row_ptr = 0; block_row_ptr < num_rows - (block_height - 1); block_row_ptr += block_height){
#ifdef debug
        printf("Row group starting: %d\n", block_row_ptr);
#endif
        int row_starts[block_height];
        int row_ends[block_height];
        //int maxlen = 0;

        int _item_index_1 = 0;
        int _item_index_2 = row_offset[block_row_ptr];
#ifdef maxunroll
#pragma unroll
#endif
        for(int i = 0; i < block_height; i++){
            int rowptrindex = block_row_ptr + i;
            _item_index_1 = _item_index_2;
            _item_index_2 = row_offset[rowptrindex + 1];
            row_starts[i] = _item_index_1;
            row_ends[i] = _item_index_2;
#ifdef debug
            printf("Rows: %d %d %d\n", _item_index_1, _item_index_2, _item_index_2 - _item_index_1);
#endif
           // maxlen = max(maxlen, _item_index_2 - _item_index_1);
        }




        int item_index_1 = row_starts[0];
        int item_index_2 = row_ends[block_height - 1];
        int size = item_index_2 - item_index_1;
        int exit = (size % DATAUNROLL == 0) ? (size / DATAUNROLL) : (size / DATAUNROLL) + 1;

        float accumulator[block_height] = {ZERO};
        for(int j = 0; j < exit; j++){
            float sum[block_height] = {ZERO};
#pragma unroll
            for(int jj = 0; jj < DATAUNROLL; jj++){
                int index = (j * DATAUNROLL + jj) + item_index_1;
                float value = (index < item_index_2) ? alpha * val[index] * x[col[index]] : ZERO;
#pragma unroll
                for(int x = 0; x < block_height; x++){
                    ugentype belongs_here = (index >= row_starts[x]) & (index < row_ends[x]);
                    sum[x] += select(ZERO, value, belongs_here);
                }
            }
#pragma unroll
            for(int x = 0; x < block_height; x++){
                float new_value = accumulator[x] + sum[x];
                accumulator[x] = new_value;
            }
        }
#pragma unroll
        for(int x = 0; x < block_height; x++){

            y[block_row_ptr + x] = accumulator[x] + (y[block_row_ptr + x] * beta);
        }
    }


#ifdef FILL_IN
    int start = num_rows - (num_rows % block_height);//((int)(num_rows / block_height)) * block_height;
    for(int i = start; i < num_rows; i++){
        float shifter[FADD_LATENCY] = {(float)0};
        float accumulator = y[i] * beta;
        int item_index_1 = row_offset[i];
        int item_index_2 = row_offset[i + 1];
        int size = item_index_2 - item_index_1;
        int exit = (size % DATAUNROLL == 0) ? (size / DATAUNROLL) : (size / DATAUNROLL) + 1;
        for(int j = 0; j < exit; j++){
            float sum = ZERO;
#pragma unroll
            for(int jj = 0; jj < DATAUNROLL; jj++){
                int index = (j * DATAUNROLL + jj) + item_index_1;
                sum += (index < item_index_2) ? alpha * val[index] * x[col[index]] : ZERO;
            }
            float new_value = shifter[0] + sum;
#pragma unroll
            for(int k = 0; k < FADD_LATENCY - 1; k++){
                shifter[k] = shifter[k + 1];
            }
            shifter[FADD_LATENCY - 1] = new_value;
        }
#pragma unroll
        for(int k = 0; k < FADD_LATENCY; k++){
            accumulator += shifter[k];
        }
        y[i] = accumulator;
    }
#endif
}
