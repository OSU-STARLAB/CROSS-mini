#include <systemc.h>
#include "../defines.h"

SC_MODULE(Fetch) {
    sc_in<bool> rst;
    sc_in<bool> clk;

    // connection with control unit
    sc_event job_start;
    sc_in<pointer_type> start_addr;
    sc_in<pointer_type> end_addr;
    
    // connection with memory unit
    //     request
    sc_in<bool> mem_ready;
    sc_out<pointer_type> mem_read_address;
    sc_event * mem_read_start;  // tell mem to read addr
    //     response
    sc_in<tensor_element> mem_res_value;
    sc_in<count_type> mem_res_index;
    sc_event mem_done;
    
    // connection with intersection (ixn) unit
    sc_fifo_out<tensor_element> values_out;
    sc_fifo_out<count_type> indices_out;
    sc_event * job_done;
    
    void fetch_main();
    
    SC_CTOR(Fetch) {
        SC_THREAD(fetch_main);
        reset_signal_is(rst, true);
        sensitive << clk.pos();
    }
    
    private:
        pointer_type curr_addr;
        pointer_type stop_addr;
};
