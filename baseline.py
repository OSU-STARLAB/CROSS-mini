#!/usr/bin/env python

from scipy import sparse
import timeit


def read_fiber(filename, size):
    with open(filename, "r") as f:
        f.readline()
        pairs = [line.split(',') for line in f]
        data = [float(p[1]) for p in pairs]
        coords = ([], [int(p[0]) for p in pairs] * size)
        for i in range(size):
            for _ in range(len(data)):
                coords[0].append(i)
        fiber = sparse.csr_matrix((data*size, coords), shape=(size, 1000000))
        return fiber


fiber_a = read_fiber("test_inputs/fiber_long_a.csf", 64)
fiber_b = sparse.csc_matrix(read_fiber("test_inputs/fiber_long_b.csf", 64).T)
test = "fiber_a.dot(fiber_b)"
n = 10000
print("Average execution time of", n, "runs:",
      timeit.timeit(test, globals=globals(), number=n)*1000 / n,
      "ms")
