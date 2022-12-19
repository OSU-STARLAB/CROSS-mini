#include "memory.h"

// All accesses are handled in parallel without blocking one another,
// even when accessing the same address. Reads and writes both happen
// immediately, but the calling thread is only notified of completion
// after the MEMORY_*_LATENCY delay finishes.

// Rather than using wait(LATENCY), I have counters defined as private
// members. This allows for a fixed number of servicing functions (just
// one for read and one for write) for a variable number of memory
// channels. There might be a way to make a dynamic number of SC_THREADs
// but I didn't pursue that method since this seemed more legible.

void Mem::read_listener() {
    wait();
    
    while (true) {
        // check if any events need to be serviced
        for (int i = 0; i < PE_COUNT*2; i++) {
            // if this event is in progress
            if (mem_read[i].triggered()) {
                // if it just started
                if (read_delays[i] == 0) {
                    uint mem_addr = read_addr[i].read();
                    read_value[i].write(contents[mem_addr]);
                    read_delays[i] = MEMORY_READ_LATENCY;
                // decrement delay counter. If zero, finish up
                } else if (--read_delays[i] == 0) {
                    mem_read[i].cancel();
                    mem_read_done[i].notify();
                }
            }
        }
        // check all events in parallel every cycle
        wait();
    }
}

void Mem::write_listener() {
    wait();
    
    while (true) {
        // check if any events need to be serviced
        for (int i = 0; i < PE_COUNT; i++) {
            // if this event is in progress
            if (mem_write[i].triggered()) {
                // if it just started
                if (write_delays[i] == 0) {
                    uint mem_addr = write_addr[i].read();
                    contents[mem_addr] = write_value[i];
                    write_delays[i] = MEMORY_WRITE_LATENCY;
                // decrement delay counter. If zero, finish up
                } else if (--write_delays[i] == 0) {
                    mem_write[i].cancel();
                    mem_write_done[i].notify();
                }
            }
        }
        // check all events in parallel every cycle
        wait();
    }
}