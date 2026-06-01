__kernel void count_primes(
    __global float *outputs,
    const unsigned int num_elems
) {
    size_t g_id = get_global_id(0);
    size_t g_size = get_global_size(0);

    float sum = 0.0;

    for (size_t i = g_id ; i < num_elems ; i += g_size) {
        float part = (((float)i + 1.0) - 0.5f) / num_elems;
        sum += (1 / (1 + part * part));
    }

    outputs[g_id] = sum;
}