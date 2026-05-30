#include <CL/cl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS  // clCreateCommandQueue
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS  // clUnloadCompiler
#define CL_TARGET_OPENCL_VERSION 300

#define CL_CHECK(_expr)                                                          \
    do {                                                                         \
        cl_int _err = _expr;                                                     \
        if (_err == CL_SUCCESS) break;                                           \
        fprintf(stderr, "OpenCL Error: '%s' returned %d!\n", #_expr, (int)_err); \
        abort();                                                                 \
    } while (0)

void* CL_CHECK_ERR(void* _val, char* msg) {
    if (_val != NULL) return _val;
    fprintf(stderr, "OpenCL Error: %s\n", msg);
    abort();
    return NULL;
}

void pfnNotify(const char *errinfo, const void *private_info, size_t cb, void *user_data) {
   fprintf(stderr, "OpenCL Error (via pfn_notify): %s\n", errinfo);
}

char* loadKernel(const char* filename, size_t* source_size) {
    if (!source_size) {
        perror("source_size");
        return NULL;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* source = (char*)malloc(sizeof(char) * (size + 1));
    if (!source) {
        fclose(file);
        perror("malloc");
        return NULL;
    }

    fread(source, sizeof(char), size, file);
    fclose(file);
    source[size] = '\0';
    *source_size = size;

    return source;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <k>\n", argv[0]);
        return 1;
    }

    int k = atoi(argv[1]);
    if (k < 0) {
        printf("k must be at least 0\n");
        return 2;
    }

    return 0;    
}