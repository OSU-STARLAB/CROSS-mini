#!/usr/bin/env python
"""
functions and types for interacting with accelerator sim
"""

import struct
import subprocess
import os
from itertools import starmap
from scipy.sparse import csr_matrix, random

TensorElement = float

SIM_EXE = "../accel_sim"
BASELINE_EXE = "./baseline.py"
FILE_LHS = "temp-lhs.csfbin"
FILE_RHS = "temp-rhs.csfbin"
FILE_RES = "temp-res.csfbin"


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
                # fix up empty jobs
                # indices = list(_indices)
                # if indices[0] == -1:
                #     indices[0] = 0
                # if indices[-1] == -1:
                #     indices[-1] = indices[-2] + 1
                # for i in range(1, len(indices)-1):
                #     if indices[i] == -1:
                #         if indices[i+1] == 1:
                #             indices[i] = 0
                #         else:
                #             indices[i] = indices[i-1] + 1
                super().__init__((data, indices, ptrs), shape=shape)
                # self.eliminate_zeros()
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
        # serialize self
        self.to_file(FILE_LHS)
        # serialize rhs
        rhs.to_file(FILE_RHS)

        # decide if we're actually gonna run the sim or just use scipy
        backend = SIM_EXE if sim else BASELINE_EXE

        # construct system() call
        if os.path.exists(FILE_RES):
            os.unlink(FILE_RES)
        log = subprocess.check_output(
                [backend, FILE_LHS, FILE_RHS, FILE_RES],
                env={"SYSTEMC_DISABLE_COPYRIGHT_MESSAGE": "1"})

        if show_log:
            print('\n'.join(log.decode('UTF-8').rsplit('\n', maxsplit=100)[-99:]))
        # get runtime from last line of backend output
        time_report = log.rsplit(b'\n', maxsplit=2)[-2]
        elapsed_ns = int(float(time_report.rsplit(b' ', maxsplit=3)[-2]))
        if show_log:
            print("backend took", elapsed_ns, "ns")

        if os.path.exists(FILE_RES):
            result = Tensor(filename=FILE_RES)
            result.sort_indices()  # make results easier to compare
            return result, elapsed_ns

        print("Backend didn't complete contraction")
        return None, None

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
    A = Tensor(filename=FILE_LHS)
    B = Tensor(filename=FILE_RHS)
    TEST_NUM = "02"
    # A = Tensor(filename=f"../test_tensors/{TEST_NUM}lhs.csfbin")
    # B = Tensor(filename=f"../test_tensors/{TEST_NUM}rhs.csfbin")
    # while True:
    #     A = Tensor(random(5, 3, density=1, format='csr'))
    #     B = Tensor(random(5, 3, density=1, format='csr'))
    C_2 = A.contract_last(B, sim=False)
    print(*C_2, "ns")
    C = A.contract_last(B)#, show_log=True)
    #     if C[0] is not None:
    #         break
    #     input("Failed. Press enter to try again")
    print(*C, "ns")

    # for file in (FILE_LHS, FILE_RHS, FILE_RES):
    #     if os.path.exists(file):
    #         os.unlink(file)
    # assert not (C[0] != C_2[0]).any()

    # T = read_csf_file_repeat("../test_inputs/fiber_a.csf", 2)
    # print(T)
    # print("\npacked form:")
    # print(T.serialize().hex())
    # with open("../test_inputs/fiber_ax2.csfbin", "wb") as f:
    #     f.write(T.serialize())
    # print(Tensor(filename="../test_inputs/fiber_ax2.csfbin"))
