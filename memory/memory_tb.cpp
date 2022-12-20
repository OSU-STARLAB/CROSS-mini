#include <systemc.h>
#include "memory.h"
#include "../defines.h"

SC_MODULE(Mem_TB) {
    Mem dut;
    
    void mem_repl();
    void mem_repl_callback_read();
    void mem_repl_callback_write();
    
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
            read_any_done |= dut.mem_read_done[i];
        }
        for (int i = 0; i < PE_COUNT; i++) {
            dut.write_addr[i](write_addr[i]);
            dut.write_value[i](write_value[i]);
            write_any_done |= dut.mem_write_done[i];
        }
        
        dut.clk(clk);
        dut.ready(ready);
        
        SC_THREAD(mem_repl);
        sensitive << clk.posedge_event();
        
        SC_THREAD(mem_repl_callback_read);
        SC_THREAD(mem_repl_callback_write);
        
        
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
        
        sc_event_or_list read_any_done;
        sc_event_or_list write_any_done;
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

void Mem_TB::mem_repl() {
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
                do {
                    cout << "channel: " << endl;
                    cin >> i;
                } while (i >= PE_COUNT*2 || i < 0);
                cout << "address: " << endl;
                cin >> addr;
                read_addr[i] = addr;
                
                dut.mem_read[i].notify(1, SC_NS);
                cout << "notified." << endl << endl;
                break;
            case 'w':  // write
                do {
                    cout << "channel: " << endl;
                    cin >> i;
                } while (i >= PE_COUNT || i < 0);
                cout << "address: " << endl;
                cin >> addr;
                write_addr[i] = addr;
                cout << "value: " << endl;
                cin >> val;
                write_value[i].write(fiber_entry(5, val));
                
                dut.mem_write[i].notify(1, SC_NS);
                cout << "notified." << endl << endl;
                break;
            case 'n':  // next
                cout << "\nnext cycle." << endl;
                wait();
                break;
        }
    }
}

void Mem_TB::mem_repl_callback_read() {
    while (true) {
        wait(read_any_done);
        for (int i = 0; i < PE_COUNT*2; i++)
            if (dut.mem_read_done[i].triggered())
                cout << "mem read " << i << " done! "
                << "Got " << read_value[i] << endl;
    }
}

void Mem_TB::mem_repl_callback_write() {
    while (true) {
        wait(write_any_done);
        for (int i = 0; i < PE_COUNT; i++)
            if (dut.mem_write_done[i].triggered())
                cout << "mem write " << i << " done!" << endl;
    }
}
