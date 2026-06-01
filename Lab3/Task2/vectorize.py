import numpy as np
from numba import vectorize, float64
import sys
import time

@vectorize([float64(float64, float64)], target='parallel', nopython=True)
def approx_pi(i, num_elem):
    nn = num_elem * num_elem
    return nn / (nn + i * i - i + 0.25)

if __name__=="__main__":
    if len(sys.argv) != 2:
        print(f"Usage: python {sys.argv[0]} <k>")
        sys.exit(1)

    k = int(sys.argv[1])

    num_elem = 1 << k

    time_start = time.perf_counter_ns()

    pi_approx = np.sum(approx_pi(np.arange(num_elem, dtype=np.float64), num_elem)) * 4.0 / num_elem

    time_end = time.perf_counter_ns()

    print(f"Approx: {pi_approx}")

    with open("measurements/approx_pi_vec.txt", "a") as f:
        f.write(f"{k} {(time_end - time_start) // 1000}\n")