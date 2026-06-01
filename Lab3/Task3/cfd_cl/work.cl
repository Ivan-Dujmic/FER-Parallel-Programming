__kernel work(
    __global float *prev,
    __global float *post,
    const int m,
    const int n,
    const int b,
    const int h,
    const int w
) {
    g_id = get_global_id(0);
    g_size = get_global_size(0);

    for (int i = 1 ; i <= m ; i += g_size) {
        for (int j = 1 ; j <= n ; j++) {
            post[i * (m + 2) + j] =
                0.25f * (
                    prev[(i - 1) * (m + 2) + j] +
                    prev[(i + 1) * (m + 2) + j] +
                    prev[i * (m + 2) + j - 1] +
                    prev[i * (m + 2) + j + 1]
                )
            ;
        }
    }
}