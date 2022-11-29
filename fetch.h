#include <systemc.h>
#include "defines.h"
#include "intersection.h"

SC_MODULE(Fetch) {
    sc_in<bool> rst;
    sc_in<bool> clk;

    // connection with control unit
    sc_in<pointer_type> start_addr;
    sc_in<pointer_type> end_addr;
    
    // connection with memory unit
    //     request
    sc_out<pointer_type> mem_req_address;
    sc_in<bool> mem_req_ready;  //  mem is ready for next address
    //     response
    sc_in<tensor_element> mem_res_value;
    sc_in<count_type> mem_res_index;
    sc_out<bool> mem_res_ready;  // I'm ready for new outputs
    sc_in<bool> mem_res_valid;  // mem's outputs are valid
    
    // connection with intersection (ixn) unit
    sc_fifo_out<tensor_element> values_out;
    sc_fifo_out<count_type> indices_out;
    sc_out<bool> done;
    
    void fetch_main();
    
    SC_CTOR(Fetch) {
        SC_THREAD(fetch_main);
        reset_signal_is(rst, true);
        sensitive << clk.pos();
    }
};
