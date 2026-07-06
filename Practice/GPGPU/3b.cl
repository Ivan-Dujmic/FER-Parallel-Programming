/*
Zadan je niz od N elemenata tipa float.
Potrebno je implementirati kernel koji računa parcijalne sume dijelova niza.
Pretpostavite da vrijedi N = x * 2^k, te da je broj dretvi po bloku 2^k.
Svaki blok treba izračunati jednu parcijalnu sumu i zapisati je u polje partial_sums.
Elemente koje obrađuje blok najprije učitati u lokalnu memoriju,
a zatim redukciju provesti unutar bloka.
*/

/*
A sequence of N elements of type float is given.
It is necessary to implement a kernel that computes the partial sums of segments of the array.
Assume that N = x × 2^k, and that the number of threads per block is 2^k.
Each block should compute one partial sum and store it in the partial_sums array.
First load the elements processed by the block into local memory,
and then perform the reduction within the block.
*/

__kernel void foo(__global const float* data, __global float* sums, const int N) {
    int group_id = get_group_id(0);
    int group_size = get_local_size(0);
    int local_id = get_local_id(0);
    int global_id = get_global_id(0);

    __local float block[group_size]; // or some fix size if we can't do it dynamically like this
    block[local_id] = data[global_id];
    barrier(CLK_LOCAL_MEM_FENCE);

    for (int stride = group_size / 2 ; stride > 0 ; stride /= 2) {
        if (local_id < stride) {
            block[local_id] += block[local_id + stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (local_id == 0) {
        sums[group_id] = block[0];
    }
}