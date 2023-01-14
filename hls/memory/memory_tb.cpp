#include <systemc.h>
#include "memory.h"
#include "../spec/spec.h"

SC_MODULE(Mem_TB) {
    Mem dut;
    
    void mem_repl();
    void mem_repl_callback_read();
    void mem_repl_callback_write();
    
    SC_CTOR(Mem_TB) :
        dut("dut"),
        clk("clk_sig", 1, SC_NS)
    {
        for (auto & read_port : dut.read_ports) {
            read_ports.push_back(new mem::read_requester(read_port));
            read_any_done |= read_ports.back()->done;
        }
        for (auto & write_port : dut.write_ports) {
            write_ports.push_back(new mem::write_requester(write_port));
            write_any_done |= write_ports.back()->done;
        }
        
        dut.clk(clk);
        
        SC_THREAD(mem_repl);
        //sensitive << clk.posedge_event();
        
        SC_THREAD(mem_repl_callback_read);
        SC_THREAD(mem_repl_callback_write);
        
        
        /*SC_THREAD(mem_port_1);
        sensitive << clk.posedge_event();
        reset_signal_is(rst, true);*/
    }
    
    private:
        sc_clock clk;
        sc_signal<bool> rst;
        // read ports: fetching fibers
        std::vector<mem::read_requester*> read_ports;
        sc_event_or_list read_any_done;
        
        // write ports: storing results
        std::vector<mem::write_requester*> write_ports;        
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
    wait(dut.ready.posedge_event());
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
                read_ports[i]->addr = addr;
                
                dut.read_ports[i].execute.notify(1, SC_NS);
                cout << "notified." << endl << endl;
                break;
            case 'w':  // write
                do {
                    cout << "channel: " << endl;
                    cin >> i;
                } while (i >= PE_COUNT || i < 0);
                cout << "address: " << endl;
                cin >> addr;
                write_ports[i]->addr = addr;
                cout << "value: " << endl;
                cin >> val;
                write_ports[i]->value.write(fiber_entry(5, val));
                
                dut.write_ports[i].execute.notify(1, SC_NS);
                cout << "notified." << endl << endl;
                break;
            case 'n':  // next
                cout << "\nnext cycle." << endl;
                wait(1, SC_NS);
                break;
        }
    }
}

void Mem_TB::mem_repl_callback_read() {
    while (true) {
        wait(read_any_done);
        cout << "###########" << endl;
        for (auto read_port : read_ports)
            if (read_port->done.triggered())
                cout << "mem read " << &read_port-&read_ports[0] << " done! "
                << "Got " << read_port->value << endl;
    }
}

void Mem_TB::mem_repl_callback_write() {
    while (true) {
        wait(write_any_done);
        cout << "***********" << endl;
        for (auto write_port : write_ports)
            if (write_port->done.triggered())
                cout << "mem write " << &write_port-&write_ports[0]
                << " done!" << endl;
    }
}
