#include <systemc.h>
#include "pe_tb.h"

int sc_main( int argc, char* argv[] ) {
	sc_report_handler::set_log_file_name("report.log");
	
	// ignore everything below this
	sc_report_handler::set_verbosity_level(SC_DEBUG);
	
	// do not print, only log. Default is: SC_LOG | SC_DISPLAY
	sc_report_handler::set_actions("tb.dut", SC_INFO, SC_LOG);
	sc_report_handler::set_actions("tb", SC_INFO, SC_LOG);
	
	PE_TB * tb = new PE_TB("tb");
	
	SC_REPORT_INFO("main", "simulation starts");
	sc_start(100, SC_NS);
	cout << "Simulation finished after " << sc_time_stamp() << endl;
	delete tb;
	return 0;
}

// TODO
void PE_TB::control() {}
void PE_TB::memory_a() {}
void PE_TB::memory_b() {}
void PE_TB::memory_c() {}
