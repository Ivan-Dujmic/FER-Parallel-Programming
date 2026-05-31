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

__kernel void count_primes(
    __global const int *inputs,
    const unsigned int size_inputs,
    __global unsigned int *count
) {
    size_t g_id = get_global_id(0);
    size_t g_size = get_global_size(0);

    unsigned int found = 0;

    for (size_t i = g_id ; i < size_inputs ; i += g_size) {
        if (is_prime(inputs[i])) {
            found++;
        }
    }

    (*count) += found;
}