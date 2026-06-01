/*
gcc approx_pi_seq.c -o build/approx_pi_seq -lm -O3
./build/approx_pi_seq <k>
*/

#define _POSIX_C_SOURCE 200809L // CLOCK_MONOTONIC

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>

static int64_t timespec_diff_us(struct timespec start, struct timespec end) {
    return (int64_t)(end.tv_sec - start.tv_sec) * 1000000LL
        + (int64_t)(end.tv_nsec - start.tv_nsec) / 1000LL;
}

float approx_pi(const unsigned int num_elems) {
    float sum = 0.0f;

    float nn = (float)num_elems * num_elems;
    float nnp = nn + 0.25f;

    for (size_t i = 0 ; i < num_elems ; i++) {
        sum += (nn / (nnp + i * i - i));
    }

    return sum;
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

    struct timespec time_begin, time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_begin);

    float pi = approx_pi(num_elem) * 4.0f / num_elem;

    clock_gettime(CLOCK_MONOTONIC, &time_end);

    FILE *file_output = fopen("measurements/approx_pi_seq.txt", "a");
    if (file_output == NULL) {
        fprintf(stderr, "fopen returned NULL\n");
        return 8;
    }

    fprintf(file_output, "%s %" PRId64 "\n",
        argv[1], // k
        timespec_diff_us(time_begin, time_end)
    );

    fclose(file_output);

    printf("Approx: %.12f\n", pi);
}