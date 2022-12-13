#include <systemc.h>
#include "fetch.h"

/*
    Fetch Unit Testbench
*/
SC_MODULE(Fetch_TB) {
    Fetch dut;
    
    // control unit interface
    sc_signal<pointer_type> start_addr;
    sc_signal<pointer_type> end_addr;
    sc_event job_start;
    
    // memory unit interface
    //     request
    sc_signal<bool> mem_ready;
    sc_signal<pointer_type> mem_read_address;
    sc_event mem_read;
    //     response
    sc_signal<tensor_element> mem_res_value;
    sc_signal<count_type> mem_res_index;
    sc_event mem_done;
    
    // intersection unit interface
    sc_fifo<fiber_entry> fiber;
    sc_event job_done;
    
    void control_source();
    void memory_source();
    void ixn_sink();
    
    std::ifstream in;
    std::ofstream out;
    
    SC_HAS_PROCESS(Fetch_TB);
    Fetch_TB(sc_module_name name, std::string inputs, std::string outputs) :
        dut("dut", job_start, job_done, mem_read, mem_done),
        clk("clk_sig", 1, SC_NS)
    {
        in.open(inputs);
        if (!in.is_open()) {
            perror(inputs.c_str());
            exit(1);
        }
        
        out.open(outputs);
        if (!out.is_open()) {
            perror(outputs.c_str());
            exit(1);
        }
        
        dut.clk(clk);
        dut.rst(rst);
        
        // control
        dut.start_addr(start_addr);
        dut.end_addr(end_addr);
        
        // mem
        dut.mem_ready(mem_ready);
        dut.mem_read_address(mem_read_address);
        dut.mem_res_value(mem_res_value);
        dut.mem_res_index(mem_res_index);
        
        
        // ixn
        dut.fiber_out(fiber);
        
        SC_THREAD(control_source);
        sensitive << clk.posedge_event();
        
        SC_THREAD(memory_source);
        sensitive << clk.posedge_event();
        
        SC_THREAD(ixn_sink);
        sensitive << clk.posedge_event();
    }
    
    ~Fetch_TB() {
        out.close();
        in.close();
        MODULE_INFO("files closed");
    }
    
    private:
        sc_clock clk;
        sc_signal<bool> rst;
};
