
#include "mix.h"

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

int* mix(int argc, char *argv[])
{	
    Config c;
    c.set();
    int** p=c.init();
    int kArrayLength = c.totalLength;
    int rank = 0, size = 0;
  //MPI_Init(&argc, &argv);
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
		      for(int i=0;i<c.num;i++){
                        ArrayCopy(p[i], count, array_buffer, 0, c.unitLength);
                        array_buffer[c.unitLength] = count;
                        MPI_Ssend(array_buffer, c.unitLength+1, MPI_INT, source, i+3, MPI_COMM_WORLD);
		      }
			
                      
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
        printf("print array c data!\n");
        for (size_t i = 0; i < kArrayLength; ++i) {
             printf("%d,", array_c[i]);
         }
        printf("\n");
	return array_c;
    } else {
      int rank = 0;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        OpenCLParameters cl_obj;
        if (!SetOpenCLParemeters(&cl_obj, c.kernelfile, c.kernelname))
        {
            return 0;
        }

        int input1[c.unitLength];
        int input2[c.unitLength] ;
        int output[c.unitLength];
	int **r;
       r=new int*[c.num];
       for(int i=0;i<c.num;i++)
	 r[i]=new int[c.unitLength];
        // cl_kernel kernel = clCreateKernel(program, "add", NULL);
        cl_mem output_buffer = clCreateBuffer(cl_obj.context,
                                              CL_MEM_WRITE_ONLY,
                                              sizeof(output),
                                              NULL,
                                              NULL);
        cl_int status = clSetKernelArg(cl_obj.kernel, c.num, sizeof(cl_mem), &output_buffer);

        MPI_Status rank_status;
        int array_buffer[c.unitLength+1];
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
	       for(int i=0;i<c.num-1;i++){
                
                MPI_Recv(array_buffer, c.unitLength+1, MPI_INT, 0, MESSAGE_SEND_DATA_B+i, MPI_COMM_WORLD, &rank_status);
                ArrayCopy(array_buffer, 0, r[i+1], 0, c.unitLength);
	       }
		
		cl_mem *cl=new cl_mem[c.num];
		for(int i=0;i<c.num;i++){
		  cl[i] = clCreateBuffer(cl_obj.context,
                                                      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                      sizeof(output),
                                                      r[i],
                                                      NULL);
		   status = clSetKernelArg(cl_obj.kernel, i, sizeof(cl_mem), &cl[i]);
//		   printf("%d clSetKernelArg\n", status);
		}

                size_t global_work_size[1] = { c.unitLength };
                status = clEnqueueNDRangeKernel(cl_obj.command_queue, cl_obj.kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
                printf("%d clEnqueueNDRangeKernel\n", status);
                status = clEnqueueReadBuffer(cl_obj.command_queue, output_buffer, CL_TRUE, 0, sizeof(output), output, 0, NULL, NULL);
		for(int i=0;i<c.num;i++)

                status = clReleaseMemObject(cl[i]);

                 printf("\n%d send data!\n", rank);

                ArrayCopy(output, 0, array_buffer, 0, c.unitLength);
                MPI_Send(array_buffer, c.unitLength+1, MPI_INT, 0, MESSAGE_RECEIVE_DATA, MPI_COMM_WORLD);
                MPI_Send(array_buffer, 0, MPI_INT, 0, MESSAGE_APPLY, MPI_COMM_WORLD);
            }
        }

        status = clReleaseMemObject(output_buffer);

        CleanOpenCLParemeters(&cl_obj);
    }
   //PI_Finalize();
 //eturn 0;
}

