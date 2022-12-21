#include <string>
#include <systemc>
#include "../memory/memory.h"
#include "../processing/pe.h"

SC_MODULE(Control_TB) {
    Mem mem;
    std::vector<PE> pes;
    
    SC_CTOR(Control_TB) :
        mem("mem"),
        clk("clk_sig", 1, SC_NS),
        rst("rst")
    {
        mem.clk(clk);
        
        for (int i = 0; i < PE_COUNT; i++) {
            std::string name = "pe";
            name.append(std::to_string(i));
            PE new_pe = new PE(name.c_str(),
                NULL, NULL,  // need job events
                mem.mem_read[i], mem.mem_read_done[i],
                mem.mem_read[i+PE_COUNT], mem.mem_read_done[i+PE_COUNT],
                mem.mem_write[i], mem.mem_write_done[i]);
            new_pe.clk(clk);
            new_pe.rst(rst);
            // connect job signals
            pes.push_back(new_pe);
        }
    }
    
    private:
        sc_clock clk;
        sc_signal<bool> rst;
        // define job signals
};