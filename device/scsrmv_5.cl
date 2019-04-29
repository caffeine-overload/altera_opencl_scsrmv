#define FADD_LATENCY 5
#ifndef DATAUNROLL
#error "please define dataunroll"
#endif

__kernel
__attribute__((reqd_work_group_size(1,1,1)))
void spmdvm (
        const int num_rows,
        const float alpha,
#ifdef CLCONST
__constant int * restrict row_offset,
                __constant int * restrict col,
                __constant float * restrict val,
                __constant float * restrict x,
#else
        __global const int * const restrict row_offset,
        __global const int * const restrict col,
        __global const float * const restrict val,
        __global const float * const restrict x,
#endif
        const float beta,
        __global       float * restrict y)
{
    //This one will be a naive SWI kernel

    for(int i = 0; i < num_rows; i++){
        float accumulator = 0.0f;
        int item_index_1 = row_offset[i];
        int item_index_2 = row_offset[i + 1];
#ifdef debug
        printf("\nData from %d to %d. y[i] is %f \n", item_index_1, item_index_2, y[i]);
#endif
        int size = item_index_2 - item_index_1;
        int exit = (size % DATAUNROLL == 0) ? (size / DATAUNROLL) : (size / DATAUNROLL) + 1;
        for(int j = 0; j < exit; j++){
            float sum = 0.0f;
#pragma unroll
            for(int jj = 0; jj < DATAUNROLL; jj++){
                int index = (j * DATAUNROLL + jj) + item_index_1;
                sum += (index < item_index_2) ? alpha * val[index] * x[col[index]] : 0.0f;
            }

            accumulator += sum;

        }

#ifdef debug
        printf("Final value for row %d is %f\n\n\n", i, accumulator);
#endif
        y[i] = accumulator + (y[i] * beta);
    }
}
