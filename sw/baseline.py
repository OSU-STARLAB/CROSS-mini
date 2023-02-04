#!/usr/bin/env python
"""
Run a sparse tensor contraction many times and get the average time
"""

import timeit
from scipy import sparse
from driver import read_csf_file_repeat

shape = (64, 1000000)
fiber_a = read_csf_file_repeat("../test_inputs/fiber_long_a.csf", *shape)
fiber_b = sparse.csc_matrix(read_csf_file_repeat("../test_inputs/fiber_long_b.csf", *shape).T)
TEST = "fiber_a.dot(fiber_b)"
N = 100
print("Average execution time of", N, "runs:",
    timeit.timeit(TEST, globals=globals(), number=N)*1000 / N,
    "ms")
