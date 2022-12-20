#include <systemc.h>
#include "store.h"

SC_MODULE(Store_TB) {
    Store dut;
    
    // control unit interface
    sc_fifo<pointer_type> destination;
    
    // connection with memory
    sc_signal<bool> mem_ready;
    sc_signal<pointer_type> mem_write_address;
    sc_signal<tensor_element> mem_write_value;
    sc_event mem_write;
    sc_event mem_done;
    
    // connection with intersection unit
    sc_fifo<tensor_element> results;
    
    void control_ixn_source();
    void memory_iface();
    
    SC_HAS_PROCESS(Store_TB);
    Store_TB(sc_module_name name) :
        dut("dut", mem_write, mem_done),
        destination(INTERSECTION_FIFO_SIZE),
        results(INTERSECTION_FIFO_SIZE),
        clk("clk_sig", 1, SC_NS)
    {
        dut.clk(clk);
        dut.rst(rst);
        
        dut.destination(destination);
        dut.results(results);
        
        dut.mem_ready(mem_ready);
        dut.mem_write_address(mem_write_address);
        dut.mem_write_value(mem_write_value);
        
        SC_THREAD(control_ixn_source);
        sensitive << clk.posedge_event();
        
        SC_THREAD(memory_iface);
        sensitive << clk.posedge_event();
    }
    
    private:
        sc_clock clk;
        sc_signal<bool> rst;
};
