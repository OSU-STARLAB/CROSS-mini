#!/usr/bin/env python
"""
Run a sparse tensor contraction many times and get the average time
"""

import timeit
import sys
from driver import Tensor
from sparse import tensordot

N = 100  # number of trials to average

if len(sys.argv) != 4:
    print(f"Usage: {sys.argv[0]} INPUTA.csfbin INPUTB.csfbin OUTPUT.csfbin")
    sys.exit(1)

tensor_a = Tensor(filename=sys.argv[1])
tensor_b = Tensor(filename=sys.argv[2])

print("Running", N, "times... ", flush=True)
TEST = "tensordot(tensor_a, tensor_b, axes=(-1, -1))"
total_sec = timeit.timeit(TEST, globals=globals(), number=N)
average_ns = int(total_sec * 1000 * 1000 * 1000 / N)  # sec->ms->us->ns
print("Baseline finishes in", average_ns, "ns")

# one last time and actually save result
cmp_axs = list(i for i in range(tensor_a.ndim+tensor_b.ndim-3))
tensor_c = tensordot(tensor_a, tensor_b, axes=(-1, -1))
tensor_c.change_compressed_axes(cmp_axs)
Tensor(tensor_c).to_file(sys.argv[3])

