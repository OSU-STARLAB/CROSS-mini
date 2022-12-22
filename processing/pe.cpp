#include "pe.h"

void PE::pe_destination_fifo() {
    while (true) {
        wait(job_start);
        destinations.write(destination);
    }
}

void PE::pe_result_combiner() {
    while (true) {
        result_combined.write(fiber_entry(
            result_indices.read(),
            result_values.read()
        ));
    }
}
