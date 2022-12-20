#include <systemc.h>
//#include "processing/intersection_tb.h"
//#include "processing/fetch_tb.h"
//#include "processing/store_tb.h"
//#include "processing/pe_tb.h"
#include "memory/memory_tb.h"

int sc_main( int argc, char* argv[] ) {
	sc_report_handler::set_log_file_name("report.log");
	
	// ignore everything below this
	sc_report_handler::set_verbosity_level(SC_DEBUG);
	
	// do not print, only log. Default is: SC_LOG | SC_DISPLAY
	sc_report_handler::set_actions("tb.dut", SC_INFO, SC_LOG);
	sc_report_handler::set_actions("tb", SC_INFO, SC_LOG);
	
	/*Intersection_TB * tb = new Intersection_TB("tb",
		"test_inputs/fiber_a.csf",
		"test_inputs/fiber_b.csf",
		"test_outputs/intersection.log"
	);*/
	
	//Fetch_TB * tb = new Fetch_TB("tb", "test_inputs/fetch_ranges.csv", "test_outputs/addresses.log");
	
	//Store_TB * tb = new Store_TB("tb");
	//PE_TB * tb = new PE_TB("tb");
	Mem_TB * tb = new Mem_TB("tb");
	
	SC_REPORT_INFO("main", "simulation starts");
	sc_start(100, SC_NS);
	cout << "Simulation finished after " << sc_time_stamp() << endl;
	delete tb;
	return 0;
}
