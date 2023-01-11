#include <systemc.h>
#include "store_tb.h"

int sc_main( int argc, char* argv[] ) {
	sc_report_handler::set_log_file_name("report.log");
	
	// ignore everything below this
	sc_report_handler::set_verbosity_level(SC_DEBUG);
	
	// do not print, only log. Default is: SC_LOG | SC_DISPLAY
	sc_report_handler::set_actions("tb.dut", SC_INFO, SC_LOG);
	sc_report_handler::set_actions("tb", SC_INFO, SC_LOG);
		
	Store_TB * tb = new Store_TB("tb");
    
	SC_REPORT_INFO("main", "simulation starts");
	sc_start(100, SC_NS);
	cout << "Simulation finished after " << sc_time_stamp() << endl;
	delete tb;
	return 0;
}

void Store_TB::control_ixn_source() {
    rst = 1;
    wait();
    rst = 0;
    wait();
    
    destination.write(30);
    results.write(fiber_entry(1, 58));
    wait();
    
    destination.write(10);
    results.write(fiber_entry(4,98));
    wait();
    
    destination.write(50);
    results.write(fiber_entry(6,-723));
    wait();
    
    destination.write(40);
    results.write(fiber_entry(9,9));
}

void Store_TB::memory_iface() {
    mem_ready = 0;
    wait(2);
    mem_ready = 1;
    
    while (true) {
        wait(mem_write);
        mem_ready = 0;
        pointer_type address = dut.mem_write_address;
        fiber_entry value = dut.mem_write_value;
        MODULE_INFO("Writing " << value << " at " << address);
        wait(3);
        mem_ready = 1;
        mem_done.notify();
    }
}