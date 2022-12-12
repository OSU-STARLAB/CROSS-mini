#include <systemc.h>
#include "../defines.h"

/*
    Intersection Unit
    
    Reads in two fibers, each as (value, index) pairs and computes a dot
    product. Notify the flush event and this module will output the accumulator
    the next time its input FIFOs are empty (actually just idxs_a, but they
    should all be evenly filled anyway).
    
    - input flush event will internally be canceled when output is emitted.
*/
SC_MODULE(Intersection) {
    sc_in<bool> rst;
    sc_in<bool> clk;
    
    sc_in<bool> done_a;
    sc_in<bool> done_b;
    
    // these stay in sync with each other
    sc_fifo_in<fiber_entry> fiber_a, fiber_b;
    sc_fifo_out<tensor_element> results;
    
    void intersection_main();
    
    SC_CTOR(Intersection) {
        SC_THREAD(intersection_main);
        reset_signal_is(rst, true);
        sensitive << clk.pos();
    }
};