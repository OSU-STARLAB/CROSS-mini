#!/usr/bin/env python
"""
Generate PDF plots to be included in paper as figures
"""
import matplotlib.pyplot as plt
import driver

# Tests to run:
#
# 1. Speedup as sparsity increases but tensor size is constant
# 2. Speedup as order increases but NNZ is constant
# 3. Speedup as size increases but NNZ and order are constant
# 4. Speedup as matrix (order-2) size increases
#

FILETYPE = "pdf"
DPI = 300
DATA_LOCATION = "../data"
PLOT_LOCATION = "../plots"
COUNT = 10  # how many tests to average and take stdev of?


def plot_1_sparsity():
    """ 1. Speedup as *sparsity* increases but tensor size is constant """
    sparsities = []
    speedups = []
    stdevs = []
    for density in (0.2, 0.15, 0.1, 0.05, 0.01):
        shape = (7, 7, 100)
        speedup, stdev = driver.accel_speedup_test(shape, shape, density, 20)
        sparsities.append(1-density)
        speedups.append(speedup)
        stdevs.append(stdev)
        print(f"speedup: {speedup:.0f}x")
    return (sparsities, speedups, stdevs), ("Sparsity", "Speedup")


def plot_2_order():
    """ 2. Speedup as order increases but NNZ is constant """
    orders = []
    speedups = []
    stdevs = []
    nnz_target = 1000
    free_dim_size = 2
    shape_list = [1000]
    print("NNZ target =", nnz_target, "contraction length =", shape_list[-1])
    for size in range(2, 5):
        shape_list.insert(0, free_dim_size)
        shape = tuple(shape_list)
        volume = free_dim_size ** (size-1) * shape_list[-1]
        density = nnz_target/volume
        speedup, stdev = driver.accel_speedup_test(shape, shape, density, COUNT)
        orders.append(size)
        speedups.append(speedup)
        stdevs.append(stdev)
        print(f"speedup: {speedup:.0f}x")
    return (orders, speedups, stdevs), ("Order", "Speedup")


def plot_3_size():
    """ 3. Speedup as size increases but NNZ and order are constant """
    sizes = []
    speedups = []
    stdevs = []
    # TODO
    for size in range(2, 10, 1):
        shape = (size, size, size)
        speedup, stdev = driver.accel_speedup_test(shape, shape, 0.1, COUNT)
        sizes.append(size)
        speedups.append(speedup)
        stdevs.append(stdev)
        print(f"speedup: {speedup:.0f}x")
    return (sizes, speedups, stdevs), ("Edge length", "Speedup")


def plot_4_matrix():
    """ 4. Speedup as matrix (order-2) size increases """
    sizes = []
    speedups = []
    stdevs = []
    for size in range(50, 101, 10):
        shape = (size, size)
        speedup, stdev = driver.accel_speedup_test(shape, shape, 0.1, COUNT)
        sizes.append(size)
        speedups.append(speedup)
        stdevs.append(stdev)
        print(f"speedup: {speedup:.0f}x")
    return (sizes, speedups, stdevs), ("Matrix size", "Speedup")


def table_dl_speedups():
    """ Table with deep learning-relevant workloads """
    speedups = []
    stdevs = []
    shapes = [(3, 3, 1024), (7, 7, 512)]
    densities = [0.005, 0.05, 0.10]
    for shape in shapes:
        for density in densities:
            speedup, stdev = driver.accel_speedup_test(
                shape, shape, density, COUNT)
            speedups.append(speedup)
            stdevs.append(stdev)
            print(shape, shape, density*100, speedup, "+-", stdev)
    return (speedups, stdevs)


if __name__ == "__main__":
    # table_dl_speedups()

    experiment_set = [
        # function        load file?  make plot?
        (plot_1_sparsity, True,       True),
        (plot_2_order,    True,      True),
        (plot_3_size,     True,      True),
        (plot_4_matrix,   True,      True),
    ]
    # if loading data from file, it won't actually call the benchmark function

    for exp, load_exp, plot_exp in experiment_set:
        # get the data
        DATAFILE = DATA_LOCATION + '/' + exp.__name__ + ".csv"
        PLOTFILE = PLOT_LOCATION + '/' + exp.__name__ + '.' + FILETYPE
        data = []
        if load_exp:
            print("loading", DATAFILE)
            with open(DATAFILE, "r", encoding="utf-8") as file:
                labels = file.readline().strip().split(',')
                stream_count = int(file.readline())
                for i in range(stream_count):
                    data_str = file.readline()
                    data.append(list(map(float, data_str.split(','))))
        else:
            # actually run experiment
            print("running", exp.__name__)
            data, labels = exp()
            with open(DATAFILE, "w", encoding="utf-8") as file:
                file.write(','.join(labels) + '\n')
                file.write(str(len(data)) + '\n')
                for stream in data:
                    file.write(','.join(map(str, stream)) + '\n')

        # make the plot
        if plot_exp:
            print("plotting", PLOTFILE)
            fig, ax = plt.subplots()
            for i in range(1, len(data), 2):
                ax.errorbar(data[0], data[i], yerr=data[i+1], fmt="o-", capsize=3)
            ax.set_xlabel(labels[0])
            ax.set_ylabel(labels[1])

            # save the plot
            fig.savefig(PLOTFILE, format=FILETYPE, dpi=DPI)
