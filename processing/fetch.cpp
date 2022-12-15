#include "fetch.h"

void Fetch::fetch_main() {
    curr_addr = 0;
    done = 0;
    
    wait();
    
    while (true) {
        // get job
        wait(job_start);
        done = 0;
        stop_addr = end_addr;
		stop_addr++;
        curr_addr = start_addr;
        MODULE_INFO("job is to read from " << curr_addr << " to " << stop_addr-1);
        wait();
        
        // run job
        while (curr_addr != stop_addr) {
            // fetch next element: request
            while (!mem_ready) wait();
            mem_read_address.write(curr_addr);
            wait(1);
            mem_read.notify();
            
            // fetch: response
            wait(mem_done);
            
            MODULE_INFO("fetched " << mem_res_value << " from " << curr_addr);
            
            // increment pointer
            curr_addr += 1;
            wait();
            
            // send element to intersection unit
            fiber_out.write(mem_res_value);
        }
        job_done.notify();
        done = 1;
        MODULE_INFO("finished job");
    }
    MODULE_INFO("finished all jobs");
}
