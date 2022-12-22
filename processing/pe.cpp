#include "pe.h"

void PE::pe_destination_fifo() {
    while (true) {
        MODULE_INFO("waiting on job_start");
        wait(job_start);
        MODULE_INFO("got a destination: " << destination);
        destinations.write(destination);
        result_indices.write(22);
    }
}

void PE::pe_result_combiner() {
    while (true) {
        MODULE_INFO("waiting on result_*");
        result_combined.write(fiber_entry(
            result_indices.read(),
            result_values.read()
        ));
        MODULE_INFO("got a result");
    }
}

void PE::job_done_notifier() {
    while (true) {
        wait(mem_done_c);
        MODULE_INFO("job_done.notify()");
        job_done.notify();
    }
}