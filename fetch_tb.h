#include <iostream>
#include <string>
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
    
    // memory unit interface
    //     request
    sc_signal<pointer_type> mem_req_address;
    sc_signal<bool> mem_req_ready;  //  mem is ready for next address
    //     response
    sc_signal<tensor_element> mem_res_value;
    sc_signal<count_type> mem_res_index;
    sc_signal<bool> mem_res_ready;  // I'm ready for new outputs
    sc_signal<bool> mem_res_valid;  // mem's outputs are valid
    
    // intersection unit interface
    sc_fifo<tensor_element> values;
    sc_fifo<count_type> indices;
    sc_signal<bool> ixn_done;
    
    void control_source();
    void memory_source();
    void ixn_sink();
    
    std::ifstream in;
    std::ofstream out;
    
    SC_HAS_PROCESS(Fetch_TB);
    Fetch_TB(sc_module_name name, std::string inputs, std::string outputs) :
        clk("clk_sig", 1, SC_NS),
        dut("dut")
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
        dut.mem_req_address(mem_req_address);
        dut.mem_req_ready(mem_req_ready);
        dut.mem_res_value(mem_res_value);
        dut.mem_res_index(mem_res_index);
        dut.mem_res_ready(mem_res_ready);
        dut.mem_res_valid(mem_res_valid);
        
        // ixn
        dut.values_out(values);
        dut.indices_out(indices);
        dut.done(ixn_done);
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