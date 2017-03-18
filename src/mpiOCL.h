/* =============================================================================
#     FileName: mpiOCL.h
#         Desc: mpi-openCL的头文件
#       Author: xiedi
#        Email: xiedimai@163.com
#     HomePage: 
#      Version: 0.0.1
#   LastChange: 2014-04-16 21:46:05
#      History:
============================================================================= */

#ifndef MPI_OPENCL_MPIOCL_H_
#define MPI_OPENCL_MPIOCL_H_

#include <mpi.h>
#include <CL/cl.h>

#include "common.h"

typedef struct OpenCLParametersStruct {
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;
    cl_device_id device;
    cl_kernel kernel;
    cl_int error_number;
} OpenCLParameters;

cl_context CreateContext();
cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device);
cl_program CreateProgram(cl_context context, cl_device_id device, const char *file_name);
void CleanUp(cl_context context, cl_command_queue command_queue, cl_program program, cl_kernel kernel);
void CleanOpenCLParemeters(OpenCLParameters *opencl_paremeters);
int SetOpenCLParemeters(OpenCLParameters *opencl_paremeters, const char *file_name, const char *kernel_name);

#endif /* MPI_OPENCL_MPIOCL_H_ */
