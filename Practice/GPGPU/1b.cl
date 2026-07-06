/*
Potrebno je implementirati
transponiranje kvadratne matrice dimenzija N × N, pri čemu je: N = 2^k.
Implementirati kernel u kojem se koristi 2D grid-stride pristup.
Pretpostavite da su dimenzije mreže i blokova zadane kao kvadratne 2D konfiguracije
te odabrane tako da je raspodjela posla među dretvama potpuno pravilna
(bez potrebe za dodatnim provjerama rubnih slučajeva).
*/

/*
It is necessary to implement
the transposition of a square matrix of size N × N, where: N = 2^k.
Implement a kernel that uses a 2D grid-stride approach.
Assume that the grid and block dimensions are specified as square 2D configurations
and are chosen such that the workload is distributed evenly among the threads
(without the need for additional boundary checks).
*/

__kernel void (__global const float* src, __global float* dst, const int N) {
    int start_col = get_global_id(0);
    int start_row = get_global_id(1);

    int stride = get_global_size(0);

    for (int r = start_row ; r < N ; r += stride) {
        for (int c = start_col ; c < N ; c += stride) {
            int input = r * N + c;
            int output = c * N + r;
            dst[output] = src[input];
        }
    }
}