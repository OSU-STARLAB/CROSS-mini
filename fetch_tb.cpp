#include "fetch_tb.h"

void Fetch_TB::control_source() {
    rst.write(1);
    wait();
    rst.write(0);
    wait();
    MODULE_INFO("source waiting");
    
    start_addr.write(5);
    end_addr.write(10);
    wait();
    dut.start_job.notify();
    MODULE_INFO("source done 1");
    wait();
    
    while (!dut.done) wait();
    
    wait(3);
    
    start_addr.write(3);
    end_addr.write(6);
    wait();
    dut.start_job.notify();
    MODULE_INFO("source done 2");
    wait();
    
    while (!dut.done) wait();
        
    start_addr.write(30);
    end_addr.write(30);
    wait();
    dut.start_job.notify();
    MODULE_INFO("source done 3");
}

void Fetch_TB::memory_source() {}

void Fetch_TB::ixn_sink() {
    wait(2);
    
    while (!dut.done) {
        wait();
    }
    MODULE_INFO("sink done!");
}
