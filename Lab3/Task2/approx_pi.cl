__kernel void approx_pi(
    __global float *outputs,
    const unsigned int num_elems
) {
    size_t g_id = get_global_id(0);
    size_t g_size = get_global_size(0);

    float sum = 0.0f;

    float nn = (float)num_elems * num_elems;
    float nnp = nn + 0.25f;

    for (size_t i = g_id ; i < num_elems ; i += g_size) {
        sum += (nn / (nnp + (float)i * i - i));
    }

    outputs[g_id] = sum;
}