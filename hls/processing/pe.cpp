#include "pe.h"

void PE::pe_destination_fifo() {
    while (true) {
        MODULE_INFO("waiting on job_start");
        wait(job_start);
        MODULE_INFO("got job_start with destination: " << destination);
        destinations.write(destination);
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

void PE::pe_running_setter() {
	while (true) {
		running = false;
		MODULE_INFO("PE not running");
		wait(job_start);
		running = true;
		MODULE_INFO("PE running");
		wait(job_done);
	}
}
