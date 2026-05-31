/*
gcc count_primes.c -o build/count_primes -lOpenCL
RUSTICL_ENABLE=radeonsi ./build/count_primes <k> <atomic: t/f> <rand/seq> [seed]
*/

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS  // clCreateCommandQueue
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS  // clUnloadCompiler
#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

// CONFIG
#define NUM_BLOCKS 256
#define SIZE_BLOCK 512
// CONFIG

// Takes some cl expression that returns cl_int and prints it, along it's error, and aborts if it fails 
#define CL_CHECK(_expr) \
    do { \
        cl_int _err = _expr; \
        if (_err == CL_SUCCESS) break; \
        fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err); \
        abort(); \
    } while (0)

// Checks a returned pointer/object handle and prints it's error, and aborts if it fails
void* CL_CHECK_ERR(void *_val, char *msg) {
    if (_val != NULL) return _val;
    fprintf(stderr, "OpenCL Error: %s\n", msg);
    abort();
    return NULL;
}

// Error callback function
void pfn_notify(const char *errinfo, const void *private_info, size_t cb, void *user_data) {
   fprintf(stderr, "OpenCL Error (via pfn_notify): %s\n", errinfo);
}

// Loads kernel code from filename
// Returns it as char* and stores size into source_size
char* load_kernel(const char *filename, size_t *source_size) {
    if (!filename) {
        fprintf(stderr, "filename is NULL\n");
        return NULL;
    }

    if (!source_size) {
        fprintf(stderr, "source_size is NULL\n");
        return NULL;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "fopen returned NULL\n");
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "fseek failed\n");
        fclose(file);
        return NULL;
    }

    long size = ftell(file);
    if (size < 0) {
        fprintf(stderr, "ftell failed\n");
    }

    rewind(file);

    char* source = malloc(sizeof(char) * (size + 1));
    if (!source) {
        fprintf(stderr, "malloc returned NULL\n");
        fclose(file);
        return NULL;
    }

    if (fread(source, sizeof(char), size, file) != (size_t)size){
        fprintf(stderr, "fread didn't read size bytes\n");
        fclose(file);
        free(source);
        return NULL;
    }

    source[size] = '\0';
    *source_size = (size_t)size;
    
    fclose(file);
    return source;
}

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 5) {
        fprintf(stderr, "Usage: %s <k> <atomic: t/f> <rand/seq> [seed]\n", argv[0]);
        return 1;
    }

    int k = atoi(argv[1]);
    if (k < 0) {
        fprintf(stderr, "k must be at least 0\n");
        return 2;
    }

    int atomic;
    if ((strcmp(argv[2], "t") == 0) || (strcmp(argv[2], "T") == 0) || (strcmp(argv[2], "1") == 0)) {
        atomic = 1;
    } else if ((strcmp(argv[2], "f") == 0) || (strcmp(argv[2], "F") == 0) || (strcmp(argv[2], "0") == 0)) {
        atomic = 0;
    } else {
        fprintf(stderr, "atomic option must be 't' or 'f'\n");
        return 3;
    }

    int random_inputs;
    if (strcmp(argv[3], "rand") == 0) {
        random_inputs = 1;
    } else if (strcmp(argv[3], "seq") == 0) {
        random_inputs = 0;
    } else {
        fprintf(stderr, "inputs option must be \"rand\" or \"seq\"");
        return 4;
    }

    unsigned int seed;
    if (argc == 5) {
        seed = atoi(argv[4]);
    } else {
        seed = time(0);
    }

    srand(seed);

    const unsigned int size_input = 1 << k;

    int *inputs = (int*)malloc(size_input * sizeof(int));
    unsigned int count = 0;

    if (inputs == NULL) {
        fprintf(stderr, "Failed to allocate memory for the input array\n");
        return 5;
    }

    if (random_inputs) {
        for (unsigned int i = 0 ; i < size_input ; i++) {
            inputs[i] = rand();
        }
    } else {
        for (unsigned int i = 0 ; i < size_input ; i++) {
            inputs[i] = i + 1;
        }
    }

    cl_int _err = CL_INVALID_VALUE;

    cl_platform_id platform;
    CL_CHECK(clGetPlatformIDs(1, &platform, NULL));

    cl_device_id device;
    CL_CHECK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));

    cl_context context = (cl_context)CL_CHECK_ERR(clCreateContext(NULL, 1, &device, &pfn_notify, NULL, &_err), "context");

    const char* cl_filename = atomic
        ? "count_primes_atomic.cl"
        : "count_primes_non_atomic.cl";

    size_t source_size;
    char *source = load_kernel(cl_filename, &source_size);
    if (source == NULL) {
        fprintf(stderr, "load_kernel returned NULL\n");
        free(inputs);
        return 6;
    }
    const char *source_const = source;

    cl_program program = (cl_program)CL_CHECK_ERR(clCreateProgramWithSource(context, 1, &source_const, NULL, &_err), "program");

    if (clBuildProgram(program, 1, &device, "", NULL, NULL) != CL_SUCCESS) {
        char buffer[10240];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        fprintf(stderr, "CL Compilation Error:\n%s", buffer);
        free(inputs);
        free(source);
        return 7;
    }

    cl_command_queue queue = (cl_command_queue)CL_CHECK_ERR(clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &_err), "queue");

    cl_mem inputs_buffer = (cl_mem)CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_ONLY, size_input * sizeof(int), NULL, &_err), "inputs_buffer");
    cl_mem output_buffer = (cl_mem)CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int), NULL, &_err), "output_buffer");

    CL_CHECK(clEnqueueWriteBuffer(queue, inputs_buffer, CL_TRUE, 0, size_input * sizeof(int), inputs, 0, NULL, NULL));
    CL_CHECK(clEnqueueWriteBuffer(queue, output_buffer, CL_TRUE, 0, sizeof(unsigned int), &count, 0, NULL, NULL));

    cl_kernel kernel = (cl_kernel)CL_CHECK_ERR(clCreateKernel(program, "count_primes", &_err), "kernel");
    
    CL_CHECK(clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputs_buffer));
    CL_CHECK(clSetKernelArg(kernel, 1, sizeof(int), &size_input));
    CL_CHECK(clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_buffer));

    size_t global_work_size[1] = {NUM_BLOCKS * SIZE_BLOCK};
    size_t local_work_size[1] = {SIZE_BLOCK};

    CL_CHECK(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL));
    
    CL_CHECK(clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, sizeof(unsigned int), &count, 0, NULL, NULL));

    CL_CHECK(clReleaseMemObject(inputs_buffer));
    CL_CHECK(clReleaseMemObject(output_buffer));
    CL_CHECK(clReleaseKernel(kernel));
    CL_CHECK(clReleaseCommandQueue(queue));
    CL_CHECK(clReleaseProgram(program));
    CL_CHECK(clReleaseContext(context));
    CL_CHECK(clReleaseDevice(device));

    free(inputs);
    free(source);

    printf("Count: %u\n", count);

    return 0;    
}