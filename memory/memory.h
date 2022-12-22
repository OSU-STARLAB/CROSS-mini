#include <systemc>
#include "../defines.h"

SC_MODULE(Mem) {
    sc_in<bool> clk;
    sc_signal<bool> ready;
    
    // read ports: fetching fibers
    sc_vector<sc_in<pointer_type>> read_addr;
    sc_vector<sc_signal<fiber_entry>> read_value;
    sc_event_or_list mem_read_any;
    sc_vector<sc_event> mem_read;
    sc_vector<sc_event> mem_read_done;
    
    // write ports: storing results
    sc_vector<sc_in<pointer_type>> write_addr;
    sc_vector<sc_in<fiber_entry>> write_value;
    sc_event_or_list mem_write_any;
    sc_vector<sc_event> mem_write;
    sc_vector<sc_event> mem_write_done;
    
    void readyer();
    void read_listener();
    void write_listener();
    
    // simulation-only helper functions
    std::tuple<pointer_type,pointer_type> append_fiber(std::string filename);  // returns pointer to start
    
    SC_CTOR(Mem) :
        clk("clk"),
        ready("ready"),
        read_addr("read_addr", PE_COUNT*2),
        read_value("read_value", PE_COUNT*2),
        mem_read("mem_read", PE_COUNT*2),
        mem_read_done("mem_read_done", PE_COUNT*2),
        
        write_addr("write_addr", PE_COUNT),
        write_value("write_value", PE_COUNT),
        mem_write("mem_write", PE_COUNT),
        mem_write_done("mem_write_done", PE_COUNT),
        
        contents(MEMORY_SIZE)
    {
        for (int i = 0; i < PE_COUNT*2; i++)
            mem_read_any |= mem_read[i];
        for (int i = 0; i < PE_COUNT; i++)
            mem_write_any |= mem_write[i];
            
        SC_THREAD(readyer);
        dont_initialize();
        sensitive << clk.pos();
        
        SC_THREAD(read_listener);
        sensitive << clk.pos();
        
        SC_THREAD(write_listener);
        sensitive << clk.pos();
    }
    
    private:
        // actual memory contents. C++ vector since it's for simulation
        std::vector<sc_signal<fiber_entry>> contents;
        
        pointer_type append_idx;
};