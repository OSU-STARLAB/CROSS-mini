#include "memory.h"

// All accesses are handled in parallel without blocking one another,
// even when accessing the same address. Reads and writes both happen
// immediately, but the calling thread is only notified of completion
// after the MEMORY_*_LATENCY delay finishes.

void Mem::readyer() {
    wait(2);
    ready = 1;
    while (true)
        wait();
}

void Mem::read_listener() {
    while (true) {
        // check all events in parallel every cycle
        MODULE_INFO("waiting!");
        wait(mem_read_any);
        MODULE_INFO("woken up!");
        // check if any events need to be serviced
        for (int i = 0; i < PE_COUNT*2; i++) {
            // if this event is in progress
            if (mem_read[i].triggered()) {
                // if it just started
                uint mem_addr = read_addr[i].read();
                MODULE_INFO("read " << i << " triggered"
                    << " at addr " << mem_addr
                )
                read_value[i] = contents[mem_addr];
                mem_read_done[i].notify(MEMORY_READ_LATENCY, SC_NS);
            }
        }
    }
}

void Mem::write_listener() {
    while (true) {
        // check all events in parallel every cycle
        wait(mem_write_any);
        // check if any events need to be serviced
        for (int i = 0; i < PE_COUNT; i++) {
            // if this event is in progress
            if (mem_write[i].triggered()) {
                // if it just started
                uint mem_addr = write_addr[i].read();
                MODULE_INFO("write " << i << " triggered"
                    << " at addr " << mem_addr
                    << " with value " << write_value[i]
                )
                contents[mem_addr] = write_value[i];
                mem_write_done[i].notify(MEMORY_WRITE_LATENCY, SC_NS);
            }
        }
    }
}

std::tuple<pointer_type,pointer_type> Mem::append_fiber(std::string filename) {
    std::ifstream in;
    in.open(filename);
    if (!in.is_open()) {
        perror(filename.c_str());
        exit(1);
    }
    
    pointer_type start_idx = append_idx;
    tensor_element val;
    count_type idx;
    fiber_entry entry;
    std::string line;
    getline(in, line);  // skip column headers
    
    while (getline(in, line)) {
        line.replace(line.find(","), 1, " ");
        std::istringstream iss(line);
        iss >> idx >> val;
        entry = fiber_entry(idx, val);
        
        contents[append_idx++] = entry;
        //cout << "appended " << contents[append_idx-1] << endl;
    }
    in.close();
    return std::tuple<pointer_type,pointer_type>(start_idx,append_idx-1);
}

void Mem::print_region(pointer_type start, pointer_type end) {
    cout << "Mem:" << endl;
    for (pointer_type i = start; i <= end; i++) {
        cout << "  [" << i << "] = " << contents[i] << endl;
    }
}