#!/usr/bin/env python
"""
functions and types for interacting with accelerator sim
"""

import struct
from itertools import starmap
from scipy.sparse import csr_matrix

TensorElement = float

# little endian uint32
def _ptrpacker(data):
    return b''.join(map(struct.Struct("<I").pack, data))
def _ptrunpacker(data):
    return struct.Struct("<I").unpack(data)[0]
# little endian int32, float
def _entrypacker(indices, data):
    return b''.join(starmap(struct.Struct("<if").pack, zip(indices, data)))
def _entryunpacker(data):
    return struct.Struct("<if").unpack(data)


class Tensor(csr_matrix):
    """
    Data structure that mirrors the format sent to the accelerator
    """

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def serialize(self, offset=0):
        """
        Return the byte-wise format that can be sent directly over
        """
        data = b''
        # order, shape, entry count
        data += _ptrpacker(
            [len(self.shape)] +
            list(self.shape) +
            [len(self.data)]
        )
        # actual entries
        data += _entrypacker(self.indices, self.data)
        # pointer count, then pointers
        indptr = [len(self.indptr)] + [p + offset for p in self.indptr]
        data += _ptrpacker(indptr)
        return data

    def gen_append(self):
        """
        Return list of arguments to "append_fiber" hls memory function
        """
        return []

    def __str__(self):
        return f"data {self.data}\nindices {self.indices}\nindptr {self.indptr}"


def read_csf_file_repeat(filename: str, repeat: int, fiber_len: int = -1):
    """
    Read a single fiber from a file as a column of a sparse matrix.
	The result is a sparse CSR matrix with shape (repeat, fiber_len)

	filename:  The .csf file to read
	repeat:    How many times to repeat this column?
	fiber_len: How long is the column? It's sparse, so default is to assume
	           the last index is the highest one, but it could be higher.
    """
    with open(filename, "r", encoding="UTF-8") as file:
        # gobble header with column names
        file.readline()

        # read rest of file into memory
        pairs = [line.split(',') for line in file]

        # grab first of each pair as coord and second as data
        coords_single = [int(p[0]) for p in pairs]

		# fit matrix height to data if length unspecified
        if fiber_len < 1:
            fiber_len = max(coords_single) + 1

        coords = ([], coords_single * repeat)
        data = [TensorElement(p[1]) for p in pairs]

        # repeat single fiber many times
        for i in range(repeat):
            for _ in range(len(data)):
                coords[0].append(i)

        # convert to tensor format
        return Tensor((data*repeat, coords), shape=(repeat, fiber_len))


def read_csfbin_file(filename: str):
    """
    Read a csfbin file, which is an unambiguous file format
    described in formats.md. Returns a new Tensor
    """
    with open(filename, "rb") as file:
        order = _ptrunpacker(file.read(4))
        shape = list(map(_ptrunpacker, [file.read(4) for _ in range(order)]))
        entry_count = _ptrunpacker(file.read(4))
        entries = list(map(_entryunpacker, [file.read(8) for _ in range(entry_count)]))
        ptr_count = _ptrunpacker(file.read(4))
        ptrs = list(map(_ptrunpacker, [file.read(4) for _ in range(ptr_count)]))
        (indices, data) = zip(*entries)
        return Tensor((data, indices, ptrs), shape=shape)


if __name__ == "__main__":
    T = read_csf_file_repeat("../test_inputs/fiber_a.csf", 2)
    print(T)
    print("\npacked form:")
    print(T.serialize().hex())
    with open("../test_inputs/fiber_ax2.csfbin", "wb") as f:
        f.write(T.serialize())
    print(read_csfbin_file("../test_inputs/fiber_ax2.csfbin"))
