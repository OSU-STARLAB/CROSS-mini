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
        
        line.replace(line.find(","), 1, " ");
        std::istringstream iss(line);
        pointer_type start, end;
        iss >> start >> end;
        
        start_addr.write(start);
        end_addr.write(end);
        wait();
        dut.start_job.notify();
        MODULE_INFO("source done 1");
        wait();
        
        while (!dut.done) wait();    
    }
}

void Fetch_TB::memory_source() {}

void Fetch_TB::ixn_sink() {
    wait(2);
    
    while (!dut.done) {
        wait();
    }
    MODULE_INFO("sink done!");
}
