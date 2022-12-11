#include "fetch_tb.h"
#include <cstddef>
#include <sstream>
#include <string>

void Fetch_TB::control_source() {
    rst.write(1);
    wait();
    rst.write(0);
    wait();
    MODULE_INFO("source waiting");
    
    std::string line;
    getline(in, line);  // consume column names
    
    while (getline(in, line)) {
        if (line.empty()) break;
        
        // extract start and end from CSV
        line.replace(line.find(","), 1, " ");
        std::istringstream iss(line);
        pointer_type start, end;
        iss >> start >> end;
        
        start_addr.write(start);
        end_addr.write(end);
        wait();
        dut.job_start.notify();
        
        wait(job_done);
    }
    MODULE_INFO("source done");
}

void Fetch_TB::memory_source() {
    wait();
    
    while (true) {
        mem_ready = true;
        wait(mem_read_start);
        
        mem_ready = false;
        pointer_type ptr = mem_read_address;
        tensor_element value = ptr * 3;
        count_type index = ptr * 2;
        wait();
        
        mem_res_value.write(value);
        mem_res_index.write(index);
        wait();
        
        mem_done->notify();
    }
}

void Fetch_TB::ixn_sink() {
    wait();
    
    while (true) {
        tensor_element value = values.read();
        tensor_element index = indices.read();
        MODULE_INFO("received (" << index << "," << value << ")");
    }    
}
