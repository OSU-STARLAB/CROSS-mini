#!/usr/bin/env python
"""
Generate PDF plots to be included in paper as figures
"""
import matplotlib.pyplot as plt
import matplotlib
from matplotlib.ticker import MultipleLocator
import driver

matplotlib.rcParams['mathtext.fontset'] = 'cm'
matplotlib.rcParams['font.family'] = 'STIXGeneral'

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
COUNT = 20  # how many tests to average and take stdev of?


def plot_a_nnz():
    """ 1. Relative speed as NNZ increases and everything else is constant """
    nonzero_counts = []
    speeds = []
    stdevs = []
    shape = (5, 5, 1000)
    volume = 5*5*1000
    for NNZ in range(1000, 10001, 1000):
        density = NNZ/volume
        _, speed = driver.accel_timing_test(shape, shape, density)
        nonzero_counts.append(NNZ)
        speeds.append(speed)
        stdevs.append(0)  # sim is deterministic
        print(f"speed: {speed:.0f}ns")
    return (nonzero_counts, speeds, stdevs), ("Number of nonzero entries (NNZ)", "Normalized speed")


def plot_b_order():
    """ 2. Speedup as order increases but NNZ is constant """
    data = []
    nnz_target = 1000
    fiber_len = 1000
    shape_left = (5, fiber_len)
    volume_left = 5 * fiber_len
    ranges = [
        (20, 261, 40),  # order 2
        (3, 16, 2),  # order 3
        (2, 7, 1),  # order 4
        (2, 5, 1),  # order 5
        (2, 4, 1),  # order 6
    ]
    print("NNZ target =", nnz_target, "contraction length =", fiber_len)
    for order in range(2, len(ranges)+2):
        size = []
        speedups = []
        stdevs = []
        for free_dim_size in range(*ranges[order-2]):
            shape = tuple([free_dim_size]*(order-1) + [fiber_len])
            print("shape", shape)
            volume = (free_dim_size ** (order-1) * fiber_len) + volume_left
            density = min(nnz_target/volume, 1)
            speedup, stdev = driver.accel_speedup_test(
                    shape_left, shape, density, COUNT)
            if not speedup or not stdev:
                continue
            size.append(volume)
            speedups.append(speedup)
            stdevs.append(stdev)
            print(f"  speedup: {speedup:.0f}x")
        data.append(size)
        data.append(speedups)
        data.append(stdevs)
    return data, ("Operand tensor volume", "Speedup ($\\times$)")


def plot_c_matrix():
    """ 4. Speedup as matrix (order-2) size increases """
    sizes = []
    speedups = []
    stdevs = []
    for size in range(10, 101, 10):
        shape = (size, size)
        speedup, stdev = driver.accel_speedup_test(shape, shape, 0.01, COUNT)
        sizes.append(size)
        speedups.append(speedup)
        stdevs.append(stdev)
        print(f"speedup: {speedup:.0f}x")
    return (sizes, speedups, stdevs), ("Square matrix size", "Speedup ($\\times$)")


def table_dl_speedups():
    """ Table with deep learning-relevant workloads """
    speedups = []
    stdevs = []

    # # first half of table
    # shapes = [[(3, 3, 1024)]*2, [(7, 7, 512)]*2]
    # densities = [0.005, 0.05, 0.10]

    # # second half
    shapes = [[(32, 8, 32), (32, 8, 128, 32)],
              [(32, 8, 32, 8, 128), (32, 128)]]
    densities = [0.005, 0.01, 0.03]
    densities = [0.005]
    for shape1, shape2 in shapes:
        for density in densities:
            speedup, stdev = driver.accel_speedup_test(
                shape1, shape2, density, COUNT)
            speedups.append(speedup)
            stdevs.append(stdev)
            print(shape1, shape2, density*100, speedup, "+-", stdev)

    print("table summary:")
    idx = 0
    for shape1, shape2 in shapes:
        for density in densities:
            print(shape1, shape2, density*100, speedups[idx], "+-", stdevs[idx])
            idx += 1
    return (speedups, stdevs)


if __name__ == "__main__":
    table_dl_speedups()
    exit()
    iarest

    experiment_set = [
        # function        load file?  make plot?
        (plot_a_nnz,      True,       True),
        (plot_b_order,    True,       True),
        (plot_c_matrix,   True,       True),
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
            fig, ax = plt.subplots(figsize=(3.5, 2.5))
            ax.set_prop_cycle('color', plt.cm.tab20c.colors[4:])
            if exp == plot_a_nnz:
                print("special for A")
                for i in range(0, len(data), 3):
                    ax.plot(
                            data[i],
                            [d/data[i+1][-1] for d in data[i+1]],
                            marker="d")
                ax.xaxis.set_minor_locator(MultipleLocator(400))
                ax.yaxis.set_minor_locator(MultipleLocator(0.05))
                ax.set_ylim((0, None))

            elif exp == plot_b_order:
                print("special for B")
                ax.set_prop_cycle(
                        color=plt.cm.tab20c.colors[4:8],
                        marker=['d', '^', 'v', 'x'])
                for i in range(3, len(data), 3):
                    ax.errorbar(
                            data[i],
                            data[i+1],
                            yerr=data[i+2],
                            capsize=3,
                            markersize=4,
                            linewidth=1)
                ax.legend(["order-3", "order-4", "order-5", "order-6"],
                          loc="lower right", fancybox=False)
                ax.set_ylim((200, None))
                ax.set_xlim((0, None))
                ax.xaxis.set_minor_locator(MultipleLocator(10000))
                ax.yaxis.set_minor_locator(MultipleLocator(50))
            else:
                print("special for C")
                for i in range(0, len(data), 3):
                    ax.errorbar(
                            data[i],
                            data[i+1],
                            yerr=data[i+2],
                            fmt=".-", capsize=3)
                ax.set_yscale('log')
                ax.xaxis.set_minor_locator(MultipleLocator(2))
                ax.set_ylim((10, None))
            ax.set_xlabel(labels[0])
            ax.set_ylabel(labels[1])
            plt.tight_layout()

            # save the plot
            fig.savefig(PLOTFILE, format=FILETYPE, dpi=DPI)
