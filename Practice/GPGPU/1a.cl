__kernel void (__global const float* src, __global float* dst, const int N, const int C) {
    int block_col = get_global_id(0);
    int block_row = get_global_id(1);

    for (int i = 0 ; i < C ; i++) {
        for (int j = 0 ; j < C ; j++) {
            int input = (block_row * C + i) * N + block_col * C + j;
            int output = (block_col * C + j) * N + block_row * C + i;
            dst[output] = src[input];
        }
    }
}