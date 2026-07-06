/*
Zadan je niz od N elemenata tipa float.
Potrebno je implementirati kernel koji računa parcijalne sume dijelova niza.
Pretpostavite da vrijedi N = x * 2^k, te da je broj dretvi po bloku 2^k.
Svaki blok treba izračunati jednu parcijalnu sumu i zapisati je u polje partial_sums.
Redukciju provesti direktno korištenjem globalne memorije.
*/

/*
A sequence of N elements of type float is given.
It is necessary to implement a kernel that computes the partial sums of segments of the array.
Assume that N = x × 2^k, and that the number of threads per block is 2^k.
Each block should compute one partial sum and store it in the partial_sums array.
Perform the reduction directly using global memory.
*/

__kernel void foo(__global const float* data, __global float* sums, const int N) {
    int group_id = get_group_id(0);
    int global_id = get_global_id(0);

    atomic_add_float(&sums[group_id], data[global_index]);
}