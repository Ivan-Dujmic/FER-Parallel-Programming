/*
gcc count_primes_seq.c -o build/count_primes_seq -lm -O3
./build/count_primes_seq <k> <rand/seq> [seed]
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

int is_prime(int x) {
    if (x < 2) {
        return 0;
    }

    if (x == 2) {
        return 1;
    }

    if ((x % 2) == 0) {
        return 0;
    }

    int limit = (int)sqrt((float)x) + 1;
    for (int i = 3 ; i <= limit ; i += 2) {
        if ((x % i) == 0) {
            return 0;
        }
    }

    return 1;
}

#

unsigned int count_primes(
    const int *inputs,
    const unsigned int size_inputs
) {
    unsigned int count = 0;

    for (size_t i = 0 ; i < size_inputs ; i++) {
        if (is_prime(inputs[i])) {
            count++;
        }
    }

    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage: %s <k> <rand/seq> [seed]\n", argv[0]);
        return 1;
    }

    int k = atoi(argv[1]);
    if (k < 0) {
        fprintf(stderr, "k must be at least 0\n");
        return 2;
    }

    int random_inputs;
    if (strcmp(argv[2], "rand") == 0) {
        random_inputs = 1;
    } else if (strcmp(argv[2], "seq") == 0) {
        random_inputs = 0;
    } else {
        fprintf(stderr, "inputs option must be \"rand\" or \"seq\"");
        return 4;
    }

    unsigned int seed;
    if (argc == 4) {
        seed = atoi(argv[3]);
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

    struct timespec time_begin, time_end;
    clock_gettime(CLOCK_MONOTONIC, &time_begin);

    count = count_primes(inputs, size_input);

    clock_gettime(CLOCK_MONOTONIC, &time_end);

    FILE *file_output = fopen("measurements/count_primes_seq.txt", "a");
    if (file_output == NULL) {
        fprintf(stderr, "fopen returned NULL\n");
        return 8;
    }

    fprintf(file_output, "%s %s %" PRId64 "\n",
        argv[1], // k
        argv[2], // rand/seq
        timespec_diff_us(time_begin, time_end)
    );

    fclose(file_output);

    printf("Count: %u\n", count);
}