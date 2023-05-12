#!/usr/bin/env python
"""
functions and types for interacting with accelerator sim
"""
import timeit
import struct
import subprocess
import os
from itertools import starmap
from sparse import random, GCXS, tensordot
import numpy as np

SIM_EXE = "../accel_sim"
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


class Tensor(GCXS):
    """
    Data structure that corresponds to the format used in the accelerator
    """

    def __init__(self, *args, **kwargs):
        filename = kwargs.pop("filename", None)
        if filename:
            # Read a csfbin file, as described in formats.md
            with open(filename, "rb") as file:
                order = _ptrunpacker(file.read(4))
                shape = list(_ptrunpacker(file.read(4)) for _ in range(order))
                entry_cnt = _ptrunpacker(file.read(4))
                entries = list(
                        _entryunpacker(file.read(8)) for _ in range(entry_cnt))
                ptr_cnt = _ptrunpacker(file.read(4))
                ptrs = np.array(
                        [_ptrunpacker(file.read(4)) for _ in range(ptr_cnt)])
                (indices, data) = zip(*entries)
                # print("data", data)
                # print("indices", indices)
                # print("indptrs", ptrs)
                order = len(shape)
                cmp_axs = list(i for i in range(order-1))
                super().__init__(
                        (np.array(data), np.array(indices), ptrs),
                        shape=shape, compressed_axes=cmp_axs, prune=True)
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

    def software_contract(self, rhs):
        """ inner loop of software implementation that gets timed """
        return tensordot(self, rhs, axes=(-1, -1))

    def contract_last(self, rhs, accel=True, show_log=False):
        """
        Contract two sparse tensors by offloading to backend: sim or baseline
        """
        assert isinstance(rhs, Tensor)

        # decide if we're actually gonna run the sim or just use Python
        backend = SIM_EXE if accel else "software baseline"

        if not accel:
            test = "self.software_contract(rhs)"
            my_globals = globals()
            my_globals.update({'self': self, 'rhs': rhs})
            total_sec = timeit.timeit(test, globals=my_globals, number=1)
            # sec->ms->us->ns
            average_ns = int(total_sec * 1000 * 1000 * 1000)
            tensor_c = self.software_contract(rhs)
            return tensor_c, average_ns

        self.to_file(FILE_LHS)
        rhs.to_file(FILE_RHS)

        # construct system() call
        if os.path.exists(FILE_RES):
            os.unlink(FILE_RES)
        result = subprocess.run(
                [backend, FILE_LHS, FILE_RHS, FILE_RES],
                env={"SYSTEMC_DISABLE_COPYRIGHT_MESSAGE": "1"},
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
        if result.returncode:
            print(result.stdout.decode('UTF-8'))
            if result.stderr:
                print(result.stderr.decode('UTF-8'))
            print("Badkend didn't complete contraction")
            return None, None
        log = result.stdout

        if show_log:
            print('\n'.join(log.decode('UTF-8').rsplit('\n', maxsplit=100)[-99:]))
        # get runtime from last line of backend output
        time_report = log.rsplit(b'\n', maxsplit=2)[-2]
        elapsed_ns = int(float(time_report.rsplit(b' ', maxsplit=3)[-2]))
        if show_log:
            print("backend took", elapsed_ns, "ns")

        if os.path.exists(FILE_RES):
            result = Tensor(filename=FILE_RES)
            # result.sort_indices()  # make results easier to compare
            return result, elapsed_ns

        print("Backend didn't complete contraction")
        return None, None

    def __str__(self):
        return f"data {self.data}\nindices {self.indices}\nindptr {self.indptr}"


def accel_speedup_test(shape_a, shape_b, density, avg_of=10):
    order = len(shape_a)
    cmp_axs = list(i for i in range(order-1))
    tensor_a = Tensor(random(
        shape=shape_a, density=density,
        format="gcxs", compressed_axes=cmp_axs))
    order = len(shape_b)
    cmp_axs = list(i for i in range(order-1))
    tensor_b = Tensor(random(
        shape=shape_b, density=density,
        format="gcxs", compressed_axes=cmp_axs))

    # print(tensordot(A, B, axes=(-1, -1)).todense())
    tensor_a.contract_last(tensor_b, accel=False)
    res = []
    for i in range(avg_of):
        _, ns_sw = tensor_a.contract_last(tensor_b, accel=False)
        res.append(ns_sw)
    _, ns_hw = tensor_a.contract_last(tensor_b)
    ns_sw = np.mean(res)
    sw_stdev = np.std(np.array(res)/ns_hw)
    return ns_sw/ns_hw, sw_stdev


# Just a quick test to make sure everything is working. Also demonstrates usage
if __name__ == "__main__":
    print("=== driver test ===")
    shapes = (
        (2, 3, 2, 50),
        (3, 4, 2, 50)
    )
    warmup = 3
    if warmup:
        print("warmup", end='', flush=True)
    try:
        while True:
            speedup = accel_speedup_test(*shapes, 0.1, 100)
            if warmup:
                print(".", end='', flush=True)
                warmup -= 1
                if not warmup:
                    print("done")
                continue
            print(f"speedup: {speedup:.0f}x")
    except KeyboardInterrupt:
        print(" quit")

    # A = Tensor(filename=FILE_LHS)
    # B = Tensor(filename=FILE_RHS)
    # TEST_NUM = "04"
    # A = Tensor(filename=f"../test_tensors/{TEST_NUM}lhs.csfbin")
    # B = Tensor(filename=f"../test_tensors/{TEST_NUM}rhs.csfbin")
    # try:
    #     C_2 = A.contract_last(B, sim=False)
    #     print(*C_2, "ns")
    #     print(C_2[0].todense())
    # except Exception as e:
    #     print(e)
    # for file in (FILE_LHS, FILE_RHS, FILE_RES):
    #     if os.path.exists(file):
    #         os.unlink(file)
    # assert not (C[0] != C_2[0]).any()
