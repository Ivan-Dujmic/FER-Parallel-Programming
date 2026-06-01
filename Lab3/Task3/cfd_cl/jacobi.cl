__kernel void jacobi(
    __global const float *psi,
    __global float *psinew,
    const int m,
    const int n
) {
    int i = get_global_id(0) + 1;
    int j = get_global_id(1) + 1;

    int stride = m + 2;

    if (i <= m && j <= n) {
        psinew[i * stride + j] =
            0.25f * (
                psi[(i - 1) * stride + j] +
                psi[(i + 1) * stride + j] +
                psi[i * stride + j - 1] +
                psi[i * stride + j + 1]
            )
        ;
    }
}