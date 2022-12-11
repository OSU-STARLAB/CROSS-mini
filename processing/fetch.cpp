#include "fetch.h"

void Fetch::fetch_main() {
    curr_addr = 0;
    
    wait();
    
    while (true) {
        // get job
        wait(job_start);
        stop_addr = end_addr.read() + 1;
        curr_addr = start_addr.read();
        MODULE_INFO("job is to read from " << curr_addr << " to " << stop_addr-1);
        wait();
        
        // run job
        while (curr_addr != stop_addr) {
            // fetch next element: request
            while (!mem_ready.read()) wait();
            mem_read_address.write(curr_addr);
            wait(1);
            mem_read_start->notify();
            
            // fetch: response
            wait(mem_done);
            tensor_element elm = mem_res_value.read();
            count_type idx = mem_res_index.read();
            
            MODULE_INFO("fetched (" << idx << "," << elm << ") from " << curr_addr);
            
            // increment pointer
            curr_addr += 1;
            wait();
            
            // send element to intersection unit
            values_out.write(elm);
            indices_out.write(idx);
        }
        job_done->notify();
        MODULE_INFO("finished job");
    }
    MODULE_INFO("finished all jobs");
}