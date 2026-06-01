import numpy as np
from numba import guvectorize, float64
import sys
import time

@guvectorize([(float64[:], float64, float64[:])], '(n),()->()', target='cuda', nopython=True)
def approx_pi(block, num_elem, res):
    sum = 0.0
    nn = num_elem * num_elem

    for i in range(block.shape[0]):
        x = block[i]
        sum += nn / (nn + x * x - x + 0.25)
    res[0] = sum

if __name__=="__main__":
    if len(sys.argv) != 3:
        print(f"Usage: python {sys.argv[0]} <k> <block_size>")
        sys.exit(1)

    k = int(sys.argv[1])
    block_size = int(sys.argv[2])
    num_elem = 1 << k
    num_blocks = num_elem // block_size


    time_start = time.perf_counter_ns()

    ins = np.arange(num_elem, dtype=np.float64)
    ins = ins.reshape((num_blocks, block_size))

    results = approx_pi(ins, num_elem)

    pi_approx = np.sum(results) * 4.0 / num_elem

    time_end = time.perf_counter_ns()

    print(f"Approx: {pi_approx}")

    with open("measurements/approx_pi_guvec.txt", "a") as f:
        f.write(f"{k} {(time_end - time_start) // 1000}\n")