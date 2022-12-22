#include <systemc.h>
#include "../defines.h"

SC_MODULE(Fetch) {
    sc_in<bool> rst;
    sc_in_clk clk;

    // connection with control unit
    sc_event & job_start;
    sc_in<pointer_type> start_addr;
    sc_in<pointer_type> end_addr;
    
    // connection with memory unit
    //     request
    sc_in<bool> mem_ready;
    sc_signal<pointer_type> mem_read_address;
    sc_event & mem_read;  // tell mem to read addr
    //     response
    sc_in<fiber_entry> mem_res_value;
    sc_event & mem_done;
    
    // connection with intersection (ixn) unit
    sc_fifo_out<fiber_entry> fiber_out;
    sc_event & job_done;
    sc_signal<bool> done;
    
    void fetch_main();
    
    SC_HAS_PROCESS(Fetch);
    Fetch(sc_module_name name, sc_event & job_start, sc_event & job_done,
        sc_event & mem_read, sc_event & mem_done
    ) :
        job_start(job_start),
        mem_read(mem_read),
        mem_done(mem_done),
        job_done(job_done)
    {
        SC_THREAD(fetch_main);
        reset_signal_is(rst, true);
        sensitive << clk.pos();
    }
    
    private:
        pointer_type curr_addr;
        pointer_type stop_addr;
};
