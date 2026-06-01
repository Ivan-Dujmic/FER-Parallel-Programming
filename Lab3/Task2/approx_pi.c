/*
gcc approx_pi.c -o build/approx_pi -lOpenCL
RUSTICL_ENABLE=radeonsi ./build/approx_pi <k>
*/

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS  // clCreateCommandQueue
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS  // clUnloadCompiler
#define CL_TARGET_OPENCL_VERSION 300
#define _POSIX_C_SOURCE 200809L // CLOCK_MONOTONIC

#include <CL/cl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

// CONFIG
#define NUM_BLOCKS 512
#define SIZE_BLOCK 256
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

// Measure how long OpenCL commands take
double event_time_us(cl_event event) {
    cl_ulong begin, end;
    CL_CHECK(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(begin), &begin, NULL));
    CL_CHECK(clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL));
    return (end - begin) * 1e-3;
}

static int64_t timespec_diff_us(struct timespec start, struct timespec end) {
    return (int64_t)(end.tv_sec - start.tv_sec) * 1000000LL
        + (int64_t)(end.tv_nsec - start.tv_nsec) / 1000LL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <k>\n", argv[0]);
        return 1;
    }

    int k = atoi(argv[1]);
    if (k < 0) {
        fprintf(stderr, "k must be at least 0\n");
        return 2;
    }

    const unsigned int num_elem = 1 << k;

    const size_t num_subsums = NUM_BLOCKS * SIZE_BLOCK;

    float *outputs = (float*)malloc(num_subsums * sizeof(float));

    struct timespec time_begin, time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_begin);

    cl_int _err = CL_INVALID_VALUE;

    cl_platform_id platform;
    CL_CHECK(clGetPlatformIDs(1, &platform, NULL));

    cl_device_id device;
    CL_CHECK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));

    cl_context context = (cl_context)CL_CHECK_ERR(clCreateContext(NULL, 1, &device, &pfn_notify, NULL, &_err), "context");

    size_t source_size;
    char *source = load_kernel("approx_pi.cl", &source_size);
    if (source == NULL) {
        fprintf(stderr, "load_kernel returned NULL\n");
        free(outputs);
        return 6;
    }
    const char *source_const = source;

    cl_program program = (cl_program)CL_CHECK_ERR(clCreateProgramWithSource(context, 1, &source_const, NULL, &_err), "program");

    if (clBuildProgram(program, 1, &device, "", NULL, NULL) != CL_SUCCESS) {
        char buffer[10240];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        fprintf(stderr, "CL Compilation Error:\n%s", buffer);
        free(outputs);
        free(source);
        return 7;
    }

    cl_command_queue queue = (cl_command_queue)CL_CHECK_ERR(clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &_err), "queue");

    cl_mem outputs_buffer = (cl_mem)CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_WRITE_ONLY, num_subsums * sizeof(float), NULL, &_err), "outputs_buffer");

    cl_kernel kernel = (cl_kernel)CL_CHECK_ERR(clCreateKernel(program, "approx_pi", &_err), "kernel");
    
    CL_CHECK(clSetKernelArg(kernel, 0, sizeof(cl_mem), &outputs_buffer));
    CL_CHECK(clSetKernelArg(kernel, 1, sizeof(unsigned int), &num_elem));

    size_t global_work_size[1] = {NUM_BLOCKS * SIZE_BLOCK};
    size_t local_work_size[1] = {SIZE_BLOCK};

    CL_CHECK(clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL));
    
    CL_CHECK(clEnqueueReadBuffer(queue, outputs_buffer, CL_TRUE, 0, num_subsums * sizeof(float), outputs, 0, NULL, NULL));

    CL_CHECK(clReleaseMemObject(outputs_buffer));
    CL_CHECK(clReleaseKernel(kernel));
    CL_CHECK(clReleaseCommandQueue(queue));
    CL_CHECK(clReleaseProgram(program));
    CL_CHECK(clReleaseContext(context));
    CL_CHECK(clReleaseDevice(device));

    float approx = 0.0f;
    for (size_t i = 0 ; i < num_subsums ; i++) {
        approx += outputs[i];
    }
    approx = approx * 4.0f / num_elem;

    clock_gettime(CLOCK_MONOTONIC, &time_end);

    FILE *file_output = fopen("measurements/approx_pi.txt", "a");
    if (file_output == NULL) {
        fprintf(stderr, "fopen returned NULL\n");
        return 8;
    }

    fprintf(file_output, "%d %d %s %" PRId64 "\n",
        NUM_BLOCKS,
        SIZE_BLOCK,
        argv[1], // k
        timespec_diff_us(time_begin, time_end)
    );

    fclose(file_output);

    free(outputs);
    free(source);

    printf("Approx: %.12f\n", approx);

    return 0;    
}