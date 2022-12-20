#include <systemc.h>
#include "memory.h"
#include "../defines.h"

SC_MODULE(Mem_TB) {
    Mem dut;
    
    void mem_port_0();
    //void mem_port_1();
    
    SC_CTOR(Mem_TB) :
        dut("dut"),
        clk("clk_sig", 1, SC_NS),
        read_addr("read_addr", PE_COUNT*2),
        read_value("read_value", PE_COUNT*2),
        write_addr("write_addr", PE_COUNT),
        write_value("write_value", PE_COUNT)
    {
        for (int i = 0; i < PE_COUNT*2; i++) {
            dut.read_addr[i](read_addr[i]);
            dut.read_value[i](read_value[i]);
        }
        for (int i = 0; i < PE_COUNT; i++) {
            dut.write_addr[i](write_addr[i]);
            dut.write_value[i](write_value[i]);
        }
        
        dut.clk(clk);
        dut.ready(ready);
        
        SC_THREAD(mem_port_0);
        sensitive << clk.posedge_event();
        
        /*SC_THREAD(mem_port_1);
        sensitive << clk.posedge_event();
        reset_signal_is(rst, true);*/
    }
    
    private:
        sc_clock clk;
        sc_signal<bool> rst, ready;
        // read ports: fetching fibers
        sc_vector<sc_signal<pointer_type>> read_addr;
        sc_vector<sc_signal<fiber_entry>> read_value;
        
        // write ports: storing results
        sc_vector<sc_signal<pointer_type>> write_addr;
        sc_vector<sc_signal<fiber_entry>> write_value;
};

int sc_main( int argc, char* argv[] ) {
	//sc_report_handler::set_log_file_name("report.log");
	
	// ignore everything below this
	//sc_report_handler::set_verbosity_level(SC_DEBUG);
	
	// do not print, only log. Default is: SC_LOG | SC_DISPLAY
	//sc_report_handler::set_actions("tb.dut", SC_INFO, SC_LOG);
	//sc_report_handler::set_actions("tb", SC_INFO, SC_LOG);

	Mem_TB * tb = new Mem_TB("tb");
	
	SC_REPORT_INFO("main", "simulation starts");
	sc_start(100, SC_NS);
	cout << "Simulation finished after " << sc_time_stamp() << endl;
	delete tb;
	return 0;
}

void Mem_TB::mem_port_0() {
    MODULE_INFO("entering mem tb thread");
    rst = 0;
    wait();
    MODULE_INFO("wait 1");
    rst = 1;
    wait();
    MODULE_INFO("wait 2");
    rst = 0;
    wait();
    MODULE_INFO("entering mem tb main loop");
    
    // repl
    char cmd;
    uint i;
    uint addr;
    tensor_element val;
    while (true) {
        cout << "r to read, w to write, n for next" << endl;
        cin >> cmd;
        switch (cmd) {
            case 'r':  // read
                cout << "idx: " << endl;
                cin >> i;
                cout << "address: " << endl;
                cin >> addr;
                read_addr[i] = addr;
                
                dut.mem_read[i].notify();
                cout << "notified." << endl << endl;
                break;
            case 'w':  // write
                cout << "idx: " << endl;
                cin >> i;
                cout << "address: " << endl;
                cin >> addr;
                write_addr[i] = addr;
                cout << "value: " << endl;
                cin >> val;
                write_value[i] = fiber_entry(5, val);
                
                dut.mem_write[i].notify();
                cout << "notified." << endl << endl;
                break;
            case 'n':  // next
                cout << "\n\nnext cycle." << endl;
                wait();
                // print if any events finished
                for (int i = 0; i < PE_COUNT*2; i++) {
                    if (dut.mem_read_done[i].triggered()) {
                        dut.mem_read_done[i].cancel();
                        cout << "mem read " << i << " done! ";
                        cout << "Got " << read_value[i] << endl;
                    }
                }
                for (int i = 0; i < PE_COUNT; i++) {
                    if (dut.mem_write_done[i].triggered()) {
                        dut.mem_write_done[i].cancel();
                        cout << "mem write " << i << "done!" << endl;
                    }
                }
                break;
        }
    }
}

//void Mem_TB::mem_port_1() {}
