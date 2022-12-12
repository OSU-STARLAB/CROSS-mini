#include "fetch.h"

void Fetch::fetch_main() {
    curr_addr = 0;
    
    wait();
    
    while (true) {
        // get job
        wait(job_start);
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
            mem_read_start->notify();
            
            // fetch: response
            wait(mem_done);
            fiber_entry ent = fiber_entry(mem_res_index, mem_res_value);
            
            MODULE_INFO("fetched " << ent << " from " << curr_addr);
            
            // increment pointer
            curr_addr += 1;
            wait();
            
            // send element to intersection unit
            fiber_out.write(ent);
        }
        job_done->notify();
        MODULE_INFO("finished job");
    }
    MODULE_INFO("finished all jobs");
}
