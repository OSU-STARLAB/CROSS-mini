#include <systemc.h>
#include "../defines.h"

SC_MODULE(Store) {
    sc_in<bool> rst;
    sc_in<bool> clk;
    
    // connection with control unit
    sc_fifo_in<pointer_type> destination;
    
    // connection with intersection unit
    sc_fifo_in<tensor_element> results;
    
    // connection with memory unit
    //     request
    sc_in<bool> mem_ready;
    sc_out<pointer_type> mem_write_address;
    sc_out<tensor_element> mem_write_value;
    sc_event * mem_write_start;
    //     response
    sc_event mem_done;
    
    void store_main();
    
    SC_CTOR(Store) {
        SC_THREAD(store_main);
        reset_signal_is(rst, true);
        sensitive << clk.pos();
    }
};