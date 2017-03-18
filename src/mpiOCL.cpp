#include "mpiOCL.h"

cl_context CreateContext()
{
    cl_int error_number;
    cl_uint number_platforms;
    cl_platform_id first_platform_id;
    cl_context context = NULL;

    error_number = clGetPlatformIDs(1, &first_platform_id, &number_platforms);
    if (error_number != CL_SUCCESS || number_platforms <= 0)
    {
        std::cerr << "Failed to find any OpenCL platforms." << std::endl;
        return NULL;
    }

    cl_context_properties context_properties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)first_platform_id,
        0
    };
    context = clCreateContextFromType(context_properties,
                                      CL_DEVICE_TYPE_GPU,
                                      NULL,
                                      NULL,
                                      &error_number);
    if (error_number != CL_SUCCESS)
    {
        std::cout << "Could not create GPU context, trying CPU..." << std::endl;
        context = clCreateContextFromType(context_properties,
                                          CL_DEVICE_TYPE_CPU,
                                          NULL,
                                          NULL,
                                          &error_number);
        if (error_number != CL_SUCCESS)
        {
            std::cerr << "Failed to create an OpenCL GPU or CPU context." << std::endl;
            return NULL;
        }
    }
    return context;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device)
{
    cl_int error_number;
    cl_device_id *devices;
    cl_command_queue command_queue = NULL;
    size_t device_buffer_size = -1;

    error_number = clGetContextInfo(context,
                                    CL_CONTEXT_DEVICES,
                                    0,
                                    NULL,
                                    &device_buffer_size);
    if (error_number != CL_SUCCESS)
    {
        std::cerr << "Failed call to clGetContextInfo(..., CL_CONTEXT_DEVICES,...)";
        return NULL;
    }

    if (device_buffer_size <= 0)
    {
        std::cerr << "No devices available.";
        return NULL;
    }

    devices = new cl_device_id[device_buffer_size / sizeof(cl_device_id)];
    error_number = clGetContextInfo(context,
                                    CL_CONTEXT_DEVICES,
                                    device_buffer_size,
                                    devices,
                                    NULL);
    if (error_number != CL_SUCCESS)
    {
        delete []devices;
        std::cerr << "Failed to get device IDs!";
        return NULL;
    }

    command_queue = clCreateCommandQueue(context, devices[0], 0, NULL);
    if (command_queue == NULL)
    {
        delete []devices;
        std::cerr << "Failed to create commandQueue for device 0.";
        return NULL;
    }

    *device = devices[0];
    delete []devices;
    return command_queue;
}

cl_program CreateProgram(cl_context context, cl_device_id device, const char *file_name)
{
    cl_int error_number;
    cl_program program;

    std::ifstream kernel_file(file_name, std::ios::in);
    if (!kernel_file.is_open())
    {
        std::cerr << "Failed to open file for reading: " << file_name << std::endl;
        return NULL;
    }

    std::ostringstream oss;
    oss << kernel_file.rdbuf();

    std::string source_stand_string = oss.str();
    const char *source_string = source_stand_string.c_str();
    program = clCreateProgramWithSource(context,
                                        1,
                                        (const char**)&source_string,
                                        NULL,
                                        NULL);
    if (program == NULL)
    {
        std::cerr << "Failed to create CL program from source." << std::endl;
        return NULL;
    }

    error_number = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (error_number != CL_SUCCESS)
    {
        char build_log[16384];
        clGetProgramBuildInfo(program,
                              device,
                              CL_PROGRAM_BUILD_LOG,
                              sizeof(build_log),
                              build_log,
                              NULL);
        std::cerr << "Error in kernel: " << std::endl;
        std::cerr << build_log;
        clReleaseProgram(program);
        return NULL;
    }
    return program;
}

void CleanUp(cl_context context, cl_command_queue command_queue, cl_program program, cl_kernel kernel)
{
    if (command_queue != NULL)
    {
        clReleaseCommandQueue(command_queue);
    }
    if (kernel != NULL)
    {
        clReleaseKernel(kernel);
    }
    if (program != NULL)
    {
        clReleaseProgram(program);
    }
    if (context != NULL)
    {
        clReleaseContext(context);
    }
}

void CleanOpenCLParemeters(OpenCLParameters *opencl_paremeters)
{
    if (opencl_paremeters->command_queue != NULL)
    {
        clReleaseCommandQueue(opencl_paremeters->command_queue);
    }
    if (opencl_paremeters->kernel != NULL)
    {
        clReleaseKernel(opencl_paremeters->kernel);
    }
    if (opencl_paremeters->program != NULL)
    {
        clReleaseProgram(opencl_paremeters->program);
    }
    if (opencl_paremeters->context != NULL)
    {
        clReleaseContext(opencl_paremeters->context);
    }
}

int SetOpenCLParemeters(OpenCLParameters *opencl_paremeters,
                        const char *file_name,
                        const char *kernel_name)
{
    opencl_paremeters->context = CreateContext();
    if (opencl_paremeters->context == NULL)
    {
        return 0;
    }

    opencl_paremeters->command_queue = CreateCommandQueue(opencl_paremeters->context,
                                                          &(opencl_paremeters->device));
    if (opencl_paremeters->command_queue == NULL)
    {
        CleanOpenCLParemeters(opencl_paremeters);
        return 0;
    }

    opencl_paremeters->program = CreateProgram(opencl_paremeters->context,
                                               opencl_paremeters->device,
                                               file_name);
    if (opencl_paremeters->program == NULL)
    {
        CleanOpenCLParemeters(opencl_paremeters);
        return 0;
    }

    opencl_paremeters->kernel = clCreateKernel(opencl_paremeters->program,
                                               kernel_name,
                                               NULL);
    if (opencl_paremeters->kernel == NULL)
    {
        CleanOpenCLParemeters(opencl_paremeters);
        std::cerr << "Failed to create kernel!" << std::endl;
        return 0;
    }

    return 1;
}
