#include "pe.h"

void PE::pe_done_watcher() {
    done = false;
    //int delay = 10;
    int counter = 0;
    wait(job_start);
    while (true) {
        /*
        // wait(mem_done_c);
        for (int i = 0; i < delay; i++) {
            wait(1, SC_NS);
            if (running)
                break;
        }
        if (!running && done_internal) {
            done = true;
            MODULE_INFO("cooldown done, I'm done");
            break;
        }*/
        if (!running){// && done_internal) {
            counter++;
        }
        if (running)
            counter = 0;
        if (counter == 10) {
            done = true;
            MODULE_INFO("cooldown done, I'm done");
            break;
        }
        wait(1, SC_NS);
    }
}

void PE::pe_result_combiner() {
    done_internal = false;
    while (true) {
        MODULE_INFO("waiting on result_*");
        auto index = result_indices.read();
        if (index == -1) {
            // MODULE_INFO("Reached the end, waiting for job_done");
            //  wait(job_done);
            MODULE_INFO("setting done=true");
            done_internal = true;
            break;
        }
        auto value = result_values.read();
        result_combined.write(fiber_entry(
            index,
            value
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
    running = false;
	while (true) {
		wait(job_start);
		running = true;
		MODULE_INFO("PE running");
		wait(job_done);
        MODULE_INFO("PE not running");
        running = false;
	}
}
