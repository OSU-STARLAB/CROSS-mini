#include <systemc>
#include "../spec/spec.h"

#include "fetch.h"
#include "intersection.h"
#include "store.h"

SC_MODULE(PE) {
    sc_in<bool> rst;
    sc_in_clk clk;

    // connections with control unit
    sc_in<pointer_type> fiber_a_start;
    sc_in<pointer_type> fiber_a_end;
    sc_in<pointer_type> fiber_b_start;
    sc_in<pointer_type> fiber_b_end;
    //sc_in<pointer_type> destination;
	sc_fifo<pointer_type> destinations;
	sc_fifo<count_type> result_indices;
    sc_event & job_start;  // control tells us to start
    sc_event & job_done;  // we tell control we're done
	sc_signal<bool> running;
	sc_signal<bool> done;
	sc_signal<bool> done_internal;

    // connections with memory unit
    sc_in<bool> mem_ready;
    sc_signal<pointer_type> & mem_read_address_a;
    sc_in<fiber_entry> mem_res_value_a;  // data that's fetched
    sc_event & mem_read_a;  // we tell mem to read
    sc_event & mem_done_a;  // mem tells us it's done

    sc_signal<pointer_type> & mem_read_address_b;
    sc_in<fiber_entry> mem_res_value_b;
    sc_event & mem_read_b;
    sc_event & mem_done_b;

    sc_signal<pointer_type> & mem_write_address_c;
    sc_signal<fiber_entry> & mem_write_value_c;
    sc_event & mem_write_c;
    sc_event & mem_done_c;

    sc_signal<bool> & fetch_a_done;
    sc_signal<bool> & fetch_b_done;

    void pe_done_watcher();
    void pe_result_combiner();
    void job_done_notifier();
	void pe_running_setter();

    SC_HAS_PROCESS(PE);
    PE (sc_module_name name, sc_event & job_start, sc_event & job_done,
        sc_event & mem_read_a, sc_event & mem_done_a,
        sc_event & mem_read_b, sc_event & mem_done_b,
        sc_event & mem_write_c, sc_event & mem_done_c
    ) :
		destinations("dests", INTERSECTION_FIFO_SIZE),
        result_indices("res_idxs", INTERSECTION_FIFO_SIZE),
        job_start(job_start),
        job_done(job_done),
        mem_read_address_a(fetch_a.mem_read_address),
        mem_read_a(mem_read_a),
        mem_done_a(mem_done_a),
        mem_read_address_b(fetch_b.mem_read_address),
        mem_read_b(mem_read_b),
        mem_done_b(mem_done_b),
        mem_write_address_c(store.mem_write_address),
        mem_write_value_c(store.mem_write_value),
        mem_write_c(store.mem_write),
        mem_done_c(mem_done_c),
        fetch_a_done(fetch_a.done),
        fetch_b_done(fetch_b.done),
        fetch_a("fetch_a", job_start, mem_read_a, mem_done_a),
        fetch_b("fetch_b", job_start, mem_read_b, mem_done_b),
        fiber_a("fiber_a", INTERSECTION_FIFO_SIZE),
        fiber_b("fiber_b", INTERSECTION_FIFO_SIZE),
        ixn("ixn"),
        result_values("res_vals", INTERSECTION_FIFO_SIZE),
        result_combined("res_comb", 3),
        store("store", mem_write_c, mem_done_c)
    {
        // clock and reset
        fetch_a.clk(clk); fetch_a.rst(rst);
        fetch_b.clk(clk); fetch_b.rst(rst);
        ixn.clk(clk);     ixn.rst(rst);
        store.clk(clk);   store.rst(rst);

        // FIFO connections
        fetch_a.fiber_out(fiber_a);
        fetch_b.fiber_out(fiber_b);
        ixn.fiber_a(fiber_a);
        ixn.fiber_b(fiber_b);
        ixn.results(result_values);
        store.results(result_combined);

        // control connections
        fetch_a.start_addr(fiber_a_start);
        fetch_a.end_addr(fiber_a_end);
        fetch_b.start_addr(fiber_b_start);
        fetch_b.end_addr(fiber_b_end);
        ixn.done_a(fetch_a_done);
        ixn.done_b(fetch_b_done);
        store.destinations(destinations);

        // memory connections
        fetch_a.mem_ready(mem_ready),
        fetch_a.mem_res_value(mem_res_value_a);
        fetch_b.mem_ready(mem_ready),
        fetch_b.mem_res_value(mem_res_value_b);
        store.mem_ready(mem_ready);

        // internally there's a FIFO but externally it's a signal.
        // This thread queues them up.
        SC_THREAD(pe_done_watcher);
        SC_THREAD(pe_result_combiner);
        SC_THREAD(job_done_notifier);
		SC_THREAD(pe_running_setter);
    }

    private:
        Fetch fetch_a, fetch_b;
        sc_fifo<fiber_entry> fiber_a, fiber_b;
        Intersection ixn;
        sc_fifo<tensor_element> result_values;
        sc_fifo<fiber_entry> result_combined;
        Store store;
};
