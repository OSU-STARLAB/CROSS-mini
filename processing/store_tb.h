#include <iostream>
#include <string>
#include <systemc.h>
#include "store.h"

SC_MODULE(STORE_TB) {
    Store dut;
    
    // control unit interface
    sc_fifo<pointer_type> destination;
    
    // connection with memory
    sc_signal<bool> mem_ready;
    sc_signal<pointer_type> mem_write_address;
    sc_signal<tensor_element> mem_write_value;
    sc_event mem_write_start;
    sc_event * mem_done;
    
    // connection with intersection unit
    sc_fifo<tensor_element> results;
};