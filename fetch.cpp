#include "fetch.h"

void Fetch::fetch_main() {
    curr_addr = 0;
    mem_req_address.write(0);
    mem_res_ready.write(0);
    done.write(0);
    
    wait();
    
    while (true) {
        // get job
        wait(start_job);
        done.write(0);
        stop_addr = end_addr.read() + 1;
        curr_addr = start_addr.read();
        MODULE_INFO("job is to read from " << curr_addr << " to " << stop_addr-1);
        wait();
        
        // run job
        while (curr_addr != stop_addr) {
            // fetch next element: request
            while (!mem_req_ready.read()) wait();
            mem_req_address.write(curr_addr);
            mem_req_valid.write(1);
            
            wait();  // this might be optional
            
            // fetch: response
            mem_res_ready.write(1);
            while (!mem_res_valid.read()) wait();
            tensor_element elm = mem_res_value.read();
            count_type idx = mem_res_index.read();
            mem_req_valid.write(0);
            mem_res_ready.write(1);
            
            MODULE_INFO("fetched (" << elm << "," << "idx" << ") from " << curr_addr);
            
            // increment pointer
            curr_addr += 1;
            wait();
            
            // send element to intersection unit
            values_out.write(elm);
            indices_out.write(idx);
        }
        done.write(1);
        MODULE_INFO("finished job");
    }
    MODULE_INFO("finished all jobs");
}