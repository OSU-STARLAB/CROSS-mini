#include "store_tb.h"

void Store_TB::control_ixn_source() {
    rst = 1;
    wait();
    rst = 0;
    wait();
    
    destination.write(30);
    results.write(fiber_entry(1, 58));
    wait();
    
    destination.write(10);
    results.write(fiber_entry(4,98));
    wait();
    
    destination.write(50);
    results.write(fiber_entry(6,-723));
    wait();
    
    destination.write(40);
    results.write(fiber_entry(9,9));
}

void Store_TB::memory_iface() {
    mem_ready = 0;
    wait(2);
    mem_ready = 1;
    
    while (true) {
        wait(mem_write);
        mem_ready = 0;
        pointer_type address = dut.mem_write_address;
        fiber_entry value = dut.mem_write_value;
        MODULE_INFO("Writing " << value << " at " << address);
        wait(3);
        mem_ready = 1;
        mem_done.notify();
    }
}