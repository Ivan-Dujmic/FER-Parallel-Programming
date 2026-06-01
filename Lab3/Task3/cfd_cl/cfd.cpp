/*
g++ cfd_cl/*.cpp -o build/cfd_cl -lm -lOpenCL -O3
RUSTICL_ENABLE=radeonsi ./build/cfd_cl <scale> <numiter>
*/

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS  // clCreateCommandQueue
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS  // clUnloadCompiler
#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>

#include "boundary.h"
#include "jacobi.h"
#include "cfdio.h"

// CONFIG
#define LOCAL_X 16
#define LOCAL_Y 16
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
void* CL_CHECK_ERR(void *_val, std::string msg) {
    if (_val != NULL) return _val;
    fprintf(stderr, "OpenCL Error: %s\n", msg.c_str());
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

    char* source = (char*)malloc(sizeof(char) * (size + 1));
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

int main(int argc, char **argv)
{
	int printfreq=1000; //output frequency
	float error, bnorm;
	float tolerance=0.0; //tolerance for convergence. <=0 means do not check

	//main arrays
	float *psi;
	//temporary versions of main arrays
	float *psitmp;

	//command line arguments
	int scalefactor, numiter;

	//simulation sizes
	int bbase=10;
	int hbase=15;
	int wbase=5;
	int mbase=32;
	int nbase=32;

	int irrotational = 1, checkerr = 0;

	int m,n,b,h,w;
	int iter;
	int i,j;

	double tstart, tstop, ttot, titer;

	//do we stop because of tolerance?
	if (tolerance > 0) {checkerr=1;}

	//check command line parameters and parse them

	if (argc <3|| argc >4) {
		printf("Usage: cfd <scale> <numiter>\n");
		return 0;
	}

	scalefactor=atoi(argv[1]);
	numiter=atoi(argv[2]);

	if(!checkerr) {
		printf("Scale Factor = %i, iterations = %i\n",scalefactor, numiter);
	}
	else {
		printf("Scale Factor = %i, iterations = %i, tolerance= %g\n",scalefactor,numiter,tolerance);
	}

	printf("Irrotational flow\n");

	//Calculate b, h & w and m & n
	b = bbase*scalefactor;
	h = hbase*scalefactor;
	w = wbase*scalefactor;
	m = mbase*scalefactor;
	n = nbase*scalefactor;

	/* Values for scalefactor = 64 (good to know for parallelization)
		b = 640
		h = 960
		w = 320
		m = 2048
		n = 2048    
	*/

	printf("Running CFD on %d x %d grid in serial\n",m,n);

	//allocate arrays
	psi    = (float *) malloc((m+2)*(n+2)*sizeof(float));
	psitmp = (float *) malloc((m+2)*(n+2)*sizeof(float));

	//zero the psi array
	for (i=0;i<m+2;i++) {
		for(j=0;j<n+2;j++) {
			psi[i*(m+2)+j]=0.0;
		}
	}

	
	//set the psi boundary conditions
	// NOTE: This is quite quick, no need to do any parallelization in it
	boundarypsi(psi,m,n,b,h,w);
	
	for (i = 0; i < (m + 2) * (n + 2); i++) {
		psitmp[i] = psi[i];
	}

	//compute normalisation factor for error
	bnorm=0.0;

	// NOTE: This is quite quick, no need to do any parallelization on it
	for (i=0;i<m+2;i++) {
			for (j=0;j<n+2;j++) {
			bnorm += psi[i*(m+2)+j]*psi[i*(m+2)+j];
		}
	}
	bnorm=sqrt(bnorm);

	cl_int _err = CL_INVALID_VALUE;

    cl_platform_id platform;
    CL_CHECK(clGetPlatformIDs(1, &platform, NULL));

    cl_device_id device;
    CL_CHECK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));

    cl_context context = (cl_context)CL_CHECK_ERR(clCreateContext(NULL, 1, &device, &pfn_notify, NULL, &_err), "context");

    size_t source_size;
    char *source = load_kernel("cfd_cl/jacobi.cl", &source_size);
    if (source == NULL) {
        fprintf(stderr, "load_kernel returned NULL\n");
        free(psi);
		free(psitmp);
        return 6;
    }
    const char *source_const = source;

    cl_program program = (cl_program)CL_CHECK_ERR(clCreateProgramWithSource(context, 1, &source_const, NULL, &_err), "program");

    if (clBuildProgram(program, 1, &device, "", NULL, NULL) != CL_SUCCESS) {
        char buffer[10240];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        fprintf(stderr, "CL Compilation Error:\n%s", buffer);
        free(psi);
		free(psitmp);
        free(source);
        return 7;
    }

    cl_command_queue queue = (cl_command_queue)CL_CHECK_ERR(clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &_err), "queue");

	cl_mem buffer1 = (cl_mem)CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_WRITE, (m+2)*(n+2)*sizeof(float), NULL, &_err), "buffer1");
    cl_mem buffer2 = (cl_mem)CL_CHECK_ERR(clCreateBuffer(context, CL_MEM_READ_WRITE, (m+2)*(n+2)*sizeof(float), NULL, &_err), "buffer2");

    CL_CHECK(clEnqueueWriteBuffer(queue, buffer1, CL_TRUE, 0, (m+2)*(n+2)*sizeof(float), psi, 0, NULL, NULL));
    CL_CHECK(clEnqueueWriteBuffer(queue, buffer2, CL_TRUE, 0, (m+2)*(n+2)*sizeof(float), psitmp, 0, NULL, NULL));

    cl_kernel kernel = (cl_kernel)CL_CHECK_ERR(clCreateKernel(program, "jacobi", &_err), "kernel");

	size_t global_work_size[2] = {(size_t)m, (size_t)n};
    size_t local_work_size[2] = {LOCAL_X, LOCAL_Y};

	//begin iterative Jacobi loop
	printf("\nStarting main loop...\n\n");
	tstart=gettime();

	for(iter=1;iter<=numiter;iter++) {
		CL_CHECK(clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer1));
		CL_CHECK(clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer2));
		CL_CHECK(clSetKernelArg(kernel, 2, sizeof(int), &m));
		CL_CHECK(clSetKernelArg(kernel, 3, sizeof(int), &n));

		CL_CHECK(clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL));

		cl_mem swap = buffer1;
		buffer1 = buffer2;
		buffer2 = swap;
	}	// iter

	CL_CHECK(clEnqueueReadBuffer(queue, buffer1, CL_TRUE, 0, (m + 2) * (n + 2) * sizeof(float), psi, 0, NULL, NULL));
	CL_CHECK(clEnqueueReadBuffer(queue, buffer2, CL_TRUE, 0, (m + 2) * (n + 2) * sizeof(float), psitmp, 0, NULL, NULL));

	clFinish(queue);

	error = deltasq(psi,psitmp,m,n);
	error=sqrt(error);
	error=error/bnorm;

	if (iter > numiter) iter=numiter;

	tstop=gettime();

	ttot=tstop-tstart;
	titer=ttot/(double)iter;

	//print out some stats
	printf("\n... finished\n");
	printf("After %d iterations, the error is %g\n",iter,error);
	printf("Time for %d iterations was %g seconds\n",iter,ttot);
	printf("Each iteration took %g seconds\n",titer);

	//output results
	//writedatafiles(psi,m,n, scalefactor);
	//writeplotfile(m,n,scalefactor);

	//free un-needed arrays
	free(psi);
	free(psitmp);
	free(source);
	printf("... finished\n");

	return 0;
}
