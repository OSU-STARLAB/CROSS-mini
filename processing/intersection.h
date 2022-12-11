#include <systemc.h>
#include "../defines.h"

/*
    Intersection Unit
    
    Reads in two fibers, each as (value, index) pairs and computes a dot
    product. Notify the flush event and this module will output the accumulator
    the next time its input FIFOs are empty (actually just idxs_a, but they
    should all be evenly filled anyway).
    
    - input flush event will internally be canceled when output is emitted.
    - input fifos values_* and indices_* should be simultaneously written.
*/
SC_MODULE(Intersection) {
    sc_in<bool> rst;
    sc_in<bool> clk;
    
    sc_in<bool> done_a;
    sc_in<bool> done_b;
    
    // these stay in sync with each other
    sc_fifo<tensor_element> values_a, values_b, results;
    sc_fifo<count_type> indices_a, indices_b;
    
    void intersection_main();
    
    SC_CTOR(Intersection) : 
        values_a(INTERSECTION_FIFO_SIZE),
        values_b(INTERSECTION_FIFO_SIZE),
        indices_a(INTERSECTION_FIFO_SIZE),
        indices_b(INTERSECTION_FIFO_SIZE)
    {
        SC_THREAD(intersection_main);
        reset_signal_is(rst, true);
        sensitive << clk.pos();
    }
};