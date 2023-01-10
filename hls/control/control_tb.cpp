#include <systemc>
#include "../memory/memory.h"
#include "../processing/pe.h"

SC_MODULE(Control_TB) {
    Mem mem;
    std::vector<PE*> pes;
    
    void tb_main();
    
    SC_CTOR(Control_TB) :
        mem("mem"),
        clk("clk_sig", 1, SC_NS),
        rst("rst"),
        jobs_start("jobs_start", PE_COUNT),
        jobs_done("jobs_done", PE_COUNT),
        fiber_a_starts("fa_starts", PE_COUNT),
        fiber_a_ends("fa_ends", PE_COUNT),
        fiber_b_starts("fb_starts", PE_COUNT),
        fiber_b_ends("fb_ends", PE_COUNT),
        destinations("destinations", PE_COUNT)
    {
        mem.clk(clk);
        
        for (int i = 0; i < PE_COUNT; i++) {
            std::string name = "pe";
            name.append(std::to_string(i));
            PE * new_pe = new PE(name.c_str(),
                jobs_start[i], jobs_done[i],  // need job events
                mem.mem_read[i], mem.mem_read_done[i],
                mem.mem_read[i+PE_COUNT], mem.mem_read_done[i+PE_COUNT],
                mem.mem_write[i], mem.mem_write_done[i]
            );
            pes.push_back(new_pe);
            new_pe->clk(clk);
            new_pe->rst(rst);
            
            new_pe->fiber_a_start(fiber_a_starts[i]);
            new_pe->fiber_a_end(fiber_a_ends[i]);
            new_pe->fiber_b_start(fiber_b_starts[i]);
            new_pe->fiber_b_end(fiber_b_ends[i]);
            new_pe->destination(destinations[i]);            
            
            new_pe->mem_ready(mem.ready);
            
            mem.read_addr[i](new_pe->mem_read_address_a);
            
            mem.read_addr[i+PE_COUNT](new_pe->mem_read_address_b);
            
            new_pe->mem_res_value_a(mem.read_value[i]);
            new_pe->mem_res_value_b(mem.read_value[i+PE_COUNT]);
            
            mem.write_addr[i](new_pe->mem_write_address_c);
            mem.write_value[i](new_pe->mem_write_value_c);
        }
        
        SC_THREAD(tb_main);
    }
    
    ~Control_TB() {
        // not totally sure if this will destruct correctly
        pes.clear();
    }
    
    private:
        sc_clock clk;
        sc_signal<bool> rst;
        sc_vector<sc_event> jobs_start;
        sc_vector<sc_event> jobs_done;
        sc_vector<sc_signal<pointer_type>> fiber_a_starts;
        sc_vector<sc_signal<pointer_type>> fiber_a_ends;
        sc_vector<sc_signal<pointer_type>> fiber_b_starts;
        sc_vector<sc_signal<pointer_type>> fiber_b_ends;
        sc_vector<sc_signal<pointer_type>> destinations;
};

int sc_main(int argc, char * argv[]) {
    Control_TB * tb = new Control_TB("tb");
    sc_start(1000000, SC_NS);
    cout << "Simulation finished after " << sc_time_stamp() << endl;
    delete tb;
    return 0;
}

void Control_TB::tb_main() {
    // init phase: load up memory
    pointer_type fiber_start_a, fiber_end_a;
    pointer_type fiber_start_b, fiber_end_b;
    std::tie(fiber_start_a,fiber_end_a) = mem.append_fiber("test_inputs/fiber_long_a.csf");
    cout << "start " << fiber_start_a << " end " << fiber_end_a << endl;
    std::tie(fiber_start_b,fiber_end_b) = mem.append_fiber("test_inputs/fiber_long_b.csf");
    cout << "start " << fiber_start_b << " end " << fiber_end_b << endl;
    
    wait(1, SC_NS);  // end init
    mem.print_region(fiber_start_a, fiber_end_b);
    
    // submit job
    wait(3, SC_NS);
    
    for (int i = 0; i < PE_COUNT; i++) {
        fiber_a_starts[i] = fiber_start_a;
        fiber_a_ends[i] = fiber_end_a;
        fiber_b_starts[i] = fiber_start_b;
        fiber_b_ends[i] = fiber_end_b;
        destinations[i] = fiber_end_b + 2 + i;
        jobs_start[i].notify(1, SC_NS);
    }
    
    wait(jobs_done[0]);
    mem.print_region(fiber_start_a, fiber_end_b + PE_COUNT + 2);
    sc_stop();
}