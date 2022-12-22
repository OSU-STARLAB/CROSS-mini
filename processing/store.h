#include <systemc.h>
#include "../defines.h"

SC_MODULE(Store) {
    sc_in<bool> rst;
    sc_in<bool> clk;
    
    // connection with control unit
    sc_fifo_in<pointer_type> destination;
    
    // connection with intersection unit
    sc_fifo_in<fiber_entry> results;
    
    // connection with memory unit
    //     request
    sc_in<bool> mem_ready;
    sc_signal<pointer_type> mem_write_address;
    sc_signal<fiber_entry> mem_write_value;
    sc_event & mem_write;
    //     response
    sc_event & mem_done;
    
    void store_main();
    
    SC_HAS_PROCESS(Store);
    Store (sc_module_name name, sc_event & mem_write, sc_event & mem_done) :
        mem_write(mem_write),
        mem_done(mem_done)
    {
        SC_THREAD(store_main);
        reset_signal_is(rst, true);
        sensitive << clk.pos();
    }
};