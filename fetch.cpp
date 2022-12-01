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
        MODULE_INFO("read " << curr_addr << " to " << stop_addr-1);
        wait();
        
        // run job
        while (curr_addr != stop_addr) {
            // fetch next element
            MODULE_INFO("fetched " << curr_addr);
            
            // increment pointer
            curr_addr += 1;
            
            // send element to intersection unit
            wait();
        }
        done.write(1);
        MODULE_INFO("finished job");
    }
}