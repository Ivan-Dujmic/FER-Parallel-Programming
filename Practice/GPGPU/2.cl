/*
Zadan je niz od N cijelih brojeva u rasponu [0, B-1].
Potrebno je izračunati histogram hist[B].
Pretpostavite da je na raspolaganju proizvoljan broj dretvi.
*/

/*
A sequence of N integers in the range [0, B − 1] is given.
It is necessary to compute the histogram hist[B].
Assume that an arbitrary number of threads is available.
*/

__kernel void foo(__global const int* A, __global int* H, const int N) {
    int gid = get_global_id(0);
    int stride = get_global_size(0);

    for (int i = gid ; i < N ; i += stride) {
        atomic_inc(&H[A[i]]);
    }
}