#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "mpiOCL.h"
#include "config.h"

#define MESSAGE_EXIT 1
#define MESSAGE_APPLY 2
#define MESSAGE_SEND_DATA_A 3
#define MESSAGE_SEND_DATA_B 4
#define MESSAGE_RECEIVE_DATA 5

void ArrayCopy(int* source_array,
               const int source_begin,
               int* dest_array,
               const int dest_begin,
               const int length)
{
    assert((source_array != NULL) && (dest_array != NULL));
    for (int i = 0; i < length; ++i)
    {
        dest_array[dest_begin + i] = source_array[source_begin + i];
    }
}

int main(int argc, char *argv[])
{	
    Config c;
    c.set();
    int** p=c.init();
    int kArrayLength = c.totalLength;
    int rank = 0, size = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
 
        int array_c[kArrayLength];
        int size = 0, count =0;
        int array_buffer[c.unitLength+1] ;
        MPI_Status status;
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        int slave_numbers = size - 1;
        while (slave_numbers > 0)
        {
            MPI_Recv(array_buffer, c.unitLength+1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            switch (status.MPI_TAG)
            {
                case (MESSAGE_EXIT):
                {
                    --slave_numbers;
                    // printf("kill slave!\n");
                    break;
                }
                case (MESSAGE_APPLY):
                {
                    int source = status.MPI_SOURCE;
                    if (count < kArrayLength)
                    {
                        ArrayCopy(p[0], count, array_buffer, 0, c.unitLength);
                        array_buffer[c.unitLength] = count;
                        // printf("p[0] data!\n");
                        // for (int i = 0; i <= c.unitLength; ++i) {
                            // printf("%d,", array_buffer[i]);
                        // }
                        // printf("\n");
                        MPI_Ssend(array_buffer, c.unitLength+1, MPI_INT, source, MESSAGE_SEND_DATA_A, MPI_COMM_WORLD);
                        ArrayCopy(p[1], count, array_buffer, 0, c.unitLength);
                        array_buffer[c.unitLength] = count;
                        MPI_Ssend(array_buffer, c.unitLength+1, MPI_INT, source, MESSAGE_SEND_DATA_B, MPI_COMM_WORLD);
                        count += c.unitLength;
                        // printf("0 send data!\n");
                    } else {
                        MPI_Send(array_buffer, 0, MPI_INT, source, MESSAGE_EXIT, MPI_COMM_WORLD);
                        // printf("0 data is null!\n");
                    }
                    break;
                }
                case (MESSAGE_RECEIVE_DATA):
                {
                    ArrayCopy(array_buffer, 0, array_c, array_buffer[c.unitLength], c.unitLength);
                    // printf("0 receive data!\n");
                    break;
                }
            }
        }
        printf("print array c data!17142\n");
        for (size_t i = 0; i < kArrayLength; ++i) {
             printf("%d,", array_c[i]);
         }
        printf("\n");
    } else {
        int rank = 0;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        OpenCLParameters cl_obj;
        if (!SetOpenCLParemeters(&cl_obj, c.kernelfile, c.kernelname))
        {
            return 0;
        }

       int **r;
       r=new int*[c.num];
       for(int i=0;i<c.num;i++)
	 r[i]=new int[c.unitLength];
	int output[c.unitLength];
        // cl_kernel kernel = clCreateKernel(program, "add", NULL);
        cl_mem output_buffer = clCreateBuffer(cl_obj.context,
                                              CL_MEM_WRITE_ONLY,
                                              sizeof(output),
                                              NULL,
                                              NULL);
        cl_int status = clSetKernelArg(cl_obj.kernel, 2, sizeof(cl_mem), &output_buffer);

        MPI_Status rank_status;
        int array_buffer[c.unitLength+1] ;
        // printf("%d begin!\n", rank);
        MPI_Send(array_buffer, 0, MPI_INT, 0, MESSAGE_APPLY, MPI_COMM_WORLD);
        while (true)
        {
            MPI_Recv(array_buffer, c.unitLength+1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &rank_status);
            if (rank_status.MPI_TAG == MESSAGE_EXIT)
            {
                MPI_Send(array_buffer, 0, MPI_INT, 0, MESSAGE_EXIT, MPI_COMM_WORLD);
                break;
            } else {
                ArrayCopy(array_buffer, 0, r[0], 0, c.unitLength);
                MPI_Recv(array_buffer, c.unitLength+1, MPI_INT, 0, MESSAGE_SEND_DATA_B, MPI_COMM_WORLD, &rank_status);
                ArrayCopy(array_buffer, 0, r[1], 0, c.unitLength);

                // printf("print input 1 data!\n");
                // for (int i = 0; i < c.unitLength; ++i) {
                    // printf("%d,", input1[i]);
                // }
                // printf("\nprint input 2 data!\n");
                // for (int i = 0; i < c.unitLength; ++i) {
                    // printf("%d,", input2[i]);
                // }
                // printf("\n");
		
                cl_mem input1_buffer = clCreateBuffer(cl_obj.context,
                                                      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                      sizeof(r[0]),
                                                      r[0],
                                                      NULL);
                cl_mem input2_buffer = clCreateBuffer(cl_obj.context,
                                                      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                      sizeof(r[1]),
                                                      r[1],
                                                      NULL);

    
                status = clSetKernelArg(cl_obj.kernel, 0, sizeof(cl_mem), &input1_buffer);
                status = clSetKernelArg(cl_obj.kernel, 1, sizeof(cl_mem), &input2_buffer);

                size_t global_work_size[1] = { c.unitLength };
                status = clEnqueueNDRangeKernel(cl_obj.command_queue, cl_obj.kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

                status = clEnqueueReadBuffer(cl_obj.command_queue, output_buffer, CL_TRUE, 0, sizeof(output), output, 0, NULL, NULL);

                status = clReleaseMemObject(input1_buffer);
                status = clReleaseMemObject(input2_buffer);

                // printf("output data!\n");
                // for (int i = 0; i < c.unitLength; ++i) {
                    // printf("%d,", output[i]);
                // }
                 printf("\n%d send data!\n", rank);

                ArrayCopy(output, 0, array_buffer, 0, c.unitLength);
                MPI_Send(array_buffer, c.unitLength+1, MPI_INT, 0, MESSAGE_RECEIVE_DATA, MPI_COMM_WORLD);
                MPI_Send(array_buffer, 0, MPI_INT, 0, MESSAGE_APPLY, MPI_COMM_WORLD);
            }
        }

        status = clReleaseMemObject(output_buffer);

        CleanOpenCLParemeters(&cl_obj);
        // status = clReleaseKernel(kernel);
        // status = clReleaseProgram(program);
        // status = clReleaseCommandQueue(commandQueue);
        // status = clReleaseContext(context);
        // printf("%d end!\n", rank);
    }
    MPI_Finalize();
    return 0;
}
