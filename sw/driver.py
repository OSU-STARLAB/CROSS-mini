#!/usr/bin/env python
"""
functions and types for interacting with accelerator sim
"""

import struct
import subprocess
from itertools import starmap
from scipy.sparse import csr_matrix, random

TensorElement = float

SIM_EXE = "../accel_sim"
BASELINE_EXE = "./baseline.py"

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
    Data structure that corresponds to the format used in the accelerator
    """

    def __init__(self, *args, **kwargs):
        filename = kwargs.pop("filename", None)
        if filename:
            # Read a csfbin file, as described in formats.md
            with open(filename, "rb") as file:
                order = _ptrunpacker(file.read(4))
                shape = list(map(_ptrunpacker, [file.read(4) for _ in range(order)]))
                entry_count = _ptrunpacker(file.read(4))
                entries = list(map(_entryunpacker, [file.read(8) for _ in range(entry_count)]))
                ptr_count = _ptrunpacker(file.read(4))
                ptrs = list(map(_ptrunpacker, [file.read(4) for _ in range(ptr_count)]))
                (indices, data) = zip(*entries)
                super().__init__((data, indices, ptrs), shape=shape)
        else:
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

    def to_file(self, filename: str):
        """
        Write sparse tensor data to a file in binary form
        """
        with open(filename, "wb") as file:
            file.write(self.serialize())

    def contract_last(self, rhs, sim=True, show_log=False):
        """
        Contract two sparse tensors by offloading to backend: sim or baseline
        """
        assert isinstance(rhs, Tensor)
        name_lhs = "driver-lhs.csfbin"
        name_rhs = "driver-rhs.csfbin"
        name_res = "driver-res.csfbin"
        # serialize self
        self.to_file(name_lhs)
        # serialize rhs
        rhs.to_file(name_rhs)

        # decide if we're actually gonna run the sim or just use scipy
        BACKEND = SIM_EXE if sim else BASELINE_EXE

        # construct system() call
        log = subprocess.check_output(
                [BACKEND, name_lhs, name_rhs, name_res],
                env={"SYSTEMC_DISABLE_COPYRIGHT_MESSAGE": "1"})

        # get runtime from last line of backend output
        time_report = log.rsplit(b'\n', maxsplit=2)[-2]
        ns = int(time_report.rsplit(b' ', maxsplit=3)[-2])
        if show_log:
            print(log.decode('UTF-8'))
            print("backend took", ns, "ns")
        result = Tensor(filename=name_res)
        return result, ns

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


# Just a quick test to make sure everything is working. Also demonstrates usage
if __name__ == "__main__":
    print("Driver test:")
    # A = Tensor(filename="../test_tensors/fiber_ax2.csfbin")
    # B = Tensor(filename="../test_tensors/fiber_ax2.csfbin")
    A = Tensor(random(5, 10, density=0.9, format='csr'))
    B = Tensor(random(5, 10, density=0.9, format='csr'))
    C = A.contract_last(B, show_log=True)
    C[0].sort_indices()
    print(*C, "ns")
    C_2 = A.contract_last(B, sim=False)
    C_2[0].sort_indices()
    print(*C_2, "ns")
    # assert not (C[0] != C_2[0]).any()

    # T = read_csf_file_repeat("../test_inputs/fiber_a.csf", 2)
    # print(T)
    # print("\npacked form:")
    # print(T.serialize().hex())
    # with open("../test_inputs/fiber_ax2.csfbin", "wb") as f:
    #     f.write(T.serialize())
    # print(Tensor(filename="../test_inputs/fiber_ax2.csfbin"))
