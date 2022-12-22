#include <systemc>
#include "pe.h"

SC_MODULE(PE_TB) {
    PE dut;
    
    // control unit
    sc_signal<pointer_type> fiber_a_start;
    sc_signal<pointer_type> fiber_a_end;
    sc_signal<pointer_type> fiber_b_start;
    sc_signal<pointer_type> fiber_b_end;
    sc_signal<pointer_type> destination;
    sc_event job_start;  // control tells us to start
    sc_event job_done;  // we tell control we're done
    
    // memory unit
    sc_signal<bool> mem_ready;
    sc_signal<pointer_type> mem_read_address_a;
    sc_signal<fiber_entry> mem_res_value_a;  // data that's fetched
    sc_event mem_read_a;  // we tell mem to read
    sc_event mem_done_a;  // mem tells us it's done
    
    sc_signal<pointer_type> mem_read_address_b;
    sc_signal<fiber_entry> mem_res_value_b;
    sc_event mem_read_b;
    sc_event mem_done_b;
    
    sc_signal<pointer_type> mem_write_address_c;
    sc_signal<fiber_entry> mem_write_value_c;
    sc_event mem_write_c;
    sc_event mem_done_c;
    
    void control();
    void memory_a();
    void memory_b();
    void memory_c();
    
    SC_HAS_PROCESS(PE_TB);
    PE_TB(sc_module_name name) :
        dut("dut", job_start, job_done,
            mem_read_a, mem_done_a,
            mem_read_b, mem_done_b,
            mem_write_c, mem_done_c),
        clk("clk_sig", 1, SC_NS)
    {
        dut.clk(clk);
        dut.rst(rst);
        
        dut.fiber_a_start(fiber_a_start);
        dut.fiber_a_end(fiber_a_end);
        dut.fiber_b_start(fiber_b_start);
        dut.fiber_b_end(fiber_b_end);
        dut.destination(destination);
        
        dut.mem_ready(mem_ready);
        dut.mem_read_address_a(mem_read_address_a);
        dut.mem_res_value_a(mem_res_value_a);
        
        dut.mem_read_address_b(mem_read_address_b);
        dut.mem_res_value_b(mem_res_value_b);
        
        dut.mem_write_address_c(mem_write_address_c);
        dut.mem_write_value_c(mem_write_value_c);
        
        SC_THREAD(control);
        sensitive << clk.posedge_event();
        
        SC_THREAD(memory_a);
        sensitive << clk.posedge_event();
        
        SC_THREAD(memory_b);
        sensitive << clk.posedge_event();
        
        SC_THREAD(memory_c);
        sensitive << clk.posedge_event();
    }
    
    private:
        sc_clock clk;
        sc_signal<bool> rst;
};