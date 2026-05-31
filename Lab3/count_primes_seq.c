/*
gcc count_primes_seq.c -o build/count_primes_seq
./build/count_primes_seq <k> <rand/seq> [seed]
*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <string.h>

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

    for (int i = 3 ; i <= x / i ; i += 2) {
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

    // skip return value/error 3

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

    count = count_primes(inputs, size_input);
    printf("Count: %u\n", count);
}