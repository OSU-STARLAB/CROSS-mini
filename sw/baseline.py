#!/usr/bin/env python
"""
Run a sparse tensor contraction many times and get the average time
"""

import timeit
import sys
import scipy.sparse as sp
from driver import Tensor

N = 100  # number of trials to average

# shape = (64, 1000000)
# fiber_a = read_csf_file_repeat("../test_inputs/fiber_long_a.csf", *shape)
# fiber_b = sparse.csc_matrix(read_csf_file_repeat("../test_inputs/fiber_long_b.csf", *shape).T)
if len(sys.argv) != 4:
    print(f"Usage: {sys.argv[0]} INPUTA.csfbin INPUTB.csfbin OUTPUT.csfbin")
    sys.exit(1)

tensor_a = Tensor(filename=sys.argv[1])
tensor_b = Tensor(filename=sys.argv[2]).T

print("Running", N, "times... ", flush=True)
TEST = "tensor_a.dot(tensor_b)"
total_sec = timeit.timeit(TEST, globals=globals(), number=N)
average_ns = int(total_sec * 1000 * 1000 * 1000 / N)  # 1000x for sec->ms ms->us us->ns
print("Baseline finishes in", average_ns, "ns")

# one last time and actually save result
tensor_c = Tensor(eval(TEST))
tensor_c.to_file(sys.argv[3])

