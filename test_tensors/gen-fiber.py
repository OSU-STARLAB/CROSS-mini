#!/usr/bin/env python

import sys
import random

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} LENGTH SPARSITY > OUT.csf", file=sys.stderr)
    print("Where SPARSITY is a float from 0-1.", file=sys.stderr)
    sys.exit(1)

length = int(sys.argv[1])
sparsity = float(sys.argv[2])

if length <= 0 or sparsity < 0 or sparsity > 1:
    print("Input out of bounds", file=sys.stderr)
    sys.exit(1)

print("index,value")
for i in range(length):
    if random.random() > sparsity:
        print(f"{i},{random.randint(1,10)}")
