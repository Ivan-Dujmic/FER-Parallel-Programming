/*
Potrebno je implementirati
transponiranje kvadratne matrice dimenzija N × N, pri čemu je: N = 2^k.
Implementirati kernel u kojem svaka dretva
transponira jedan komad matrice dimenzija c × c, c = 2^l, N % c == 0
*/

/*
It is necessary to implement
the transposition of a square matrix of size N × N, where: N = 2^k.
Implement a kernel in which each thread
transposes one matrix block of size c × c, where c = 2^l and N % c == 0.
*/

__kernel void (__global const float* src, __global float* dst, const int N, const int C) {
    int block_col = get_global_id(0);
    int block_row = get_global_id(1);

    for (int r = 0 ; r < C ; r++) {
        for (int c = 0 ; c < C ; c++) {
            int input = (block_row * C + r) * N + block_col * C + c;
            int output = (block_col * C + r) * N + block_row * C + c;
            dst[output] = src[input];
        }
    }
}