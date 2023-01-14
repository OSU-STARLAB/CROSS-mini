#include <systemc>
#include "../spec/spec.h"

namespace mem {
    struct read_provider {
        sc_in<pointer_type> addr;
        sc_signal<fiber_entry> value;
        sc_event execute;
        sc_event done;
        
        read_provider(sc_signal<pointer_type> addr_sig, sc_in<fiber_entry> & value_in) {
            addr(addr_sig);
            value_in(value);
        }
        
        read_provider(const char * &) {}
    };
    
    struct read_requester {
        sc_signal<pointer_type> addr;
        sc_in<fiber_entry> value;
        sc_event & execute;
        sc_event & done;
        
        read_requester(mem::read_provider & read_port) :
            value(read_port.value),
            execute(read_port.execute),
            done(read_port.done)
        {
            read_port.addr(addr);
        }
    };
    
    struct write_provider {
        sc_in<pointer_type> addr;
        sc_in<fiber_entry> value;
        sc_event execute;
        sc_event done;
        
        write_provider(sc_signal<pointer_type> addr_sig, sc_signal<fiber_entry> value_sig) {
            addr(addr_sig);
            value(value_sig);
        }
        write_provider(const char * &) {}
    };
    
    struct write_requester {
        sc_signal<pointer_type> addr;
        sc_signal<fiber_entry> value;
        sc_event & execute;
        sc_event & done;
        
        write_requester(mem::write_provider & write_port) :
            execute(write_port.execute),
            done(write_port.done)
        {
            write_port.addr(addr);
            write_port.value(value);
        }
    };
}

SC_MODULE(Mem) {
    sc_in<bool> clk;
    sc_signal<bool> ready;
    
    // read ports: fetching fibers
    sc_vector<mem::read_provider> read_ports;
    sc_event_or_list read_any;
    
    // write ports: storing results
    sc_vector<mem::write_provider> write_ports;
    sc_event_or_list write_any;
    
    void readyer();
    void read_listener();
    void write_listener();
    
    // simulation-only helper functions
    std::tuple<pointer_type,pointer_type> append_fiber(std::string filename);  // returns pointer to start
    void print_region(pointer_type start, pointer_type end);
    
    SC_CTOR(Mem) :
        clk("clk"),
        ready("ready"),
        read_ports("read_ports", PE_COUNT*2),
        write_ports("write_ports", PE_COUNT),
        contents(MEMORY_SIZE)
    {
        for (int i = 0; i < PE_COUNT*2; i++)
            read_any |= read_ports[i].execute;
        for (int i = 0; i < PE_COUNT; i++)
            write_any |= write_ports[i].execute;
        
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