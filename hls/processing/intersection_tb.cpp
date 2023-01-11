#include <systemc.h>
#include "intersection_tb.h"
#include <random>
#include <sstream>
#include <time.h>


int sc_main( int argc, char* argv[] ) {
	sc_report_handler::set_log_file_name("report.log");

	// ignore everything below this
	sc_report_handler::set_verbosity_level(SC_DEBUG);

	// do not print, only log. Default is: SC_LOG | SC_DISPLAY
	sc_report_handler::set_actions("tb.dut", SC_INFO, SC_LOG);
	sc_report_handler::set_actions("tb", SC_INFO, SC_LOG);

	Intersection_TB * tb = new Intersection_TB("tb",
		"test_inputs/fiber_a.csf",
		"test_inputs/fiber_b.csf",
		"test_outputs/intersection.log"
	);
	
	SC_REPORT_INFO("main", "simulation starts");
	sc_start(100, SC_NS);
	cout << "Simulation finished after " << sc_time_stamp() << endl;
	delete tb;
	return 0;
}


void Intersection_TB::source_a() {
	tensor_element val;
	count_type idx;
	done_a = false;
	srand(time(NULL)); 
	
	std::string line;
	getline(in_a, line);
	
	// generate reset pulse
	rst.write(1);
	wait();
	rst.write(0);
	wait();
	
	while (getline(in_a, line)) {
		switch (rand() % 3) {
			case 0:
		 		wait(5);
		 	case 1:
				wait(2);
			case 2:
				break;
		}
		
		// read value pair from input file A
		if (fiber_a.num_free() > 0) {
			line.replace(line.find(","), 1, " ");
			std::istringstream iss(line);
			iss >> idx >> val;

			fiber_entry ent_a = fiber_entry(idx, val);
			MODULE_INFO("read from file ent_a = " << ent_a);
			
			// send to DUT
			fiber_a.write(ent_a);
			MODULE_INFO("sent to module: ent_a = " << ent_a);
		}
		wait(1);
	}
	
	wait();
	done_a = true;
	MODULE_INFO("source_a done")
}

void Intersection_TB::source_b() {
	tensor_element val;
	count_type idx;
	done_b = false;
	
	std::string line;
	getline(in_b, line);
	
	// wait for reset pulse
	wait();
	wait();
	
	while (getline(in_b, line)) {
		switch (rand() % 3) {
			case 0:
		 		wait(5);
		 	case 1:
				wait(2);
			case 2:
				break;
		}
		
		// read value pair from input file A
		if (fiber_b.num_free() > 0) {
			line.replace(line.find(","), 1, " ");
			std::istringstream iss(line);
			iss >> idx >> val;

			fiber_entry ent_b = fiber_entry(idx, val);
			MODULE_INFO("read from file ent_b = " << ent_b);
			
			// send to DUT
			fiber_b.write(ent_b);
			MODULE_INFO("sent to module: ent_b = " << ent_b);
		}
		wait(1);
	}
	
	wait();
	done_b = true;
	MODULE_INFO("source_b done")
}

void Intersection_TB::sink() {
	tensor_element output;
	output = results.read();
	out << output << endl;
	MODULE_INFO("got output " << output << " at " << sc_time_stamp());
	sc_stop();
}
