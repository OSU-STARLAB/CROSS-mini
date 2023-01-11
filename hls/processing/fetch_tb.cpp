#include <systemc.h>
#include "fetch_tb.h"
#include <cstddef>
#include <sstream>
#include <string>


int sc_main( int argc, char* argv[] ) {
	sc_report_handler::set_log_file_name("report.log");
	
	// ignore everything below this
	sc_report_handler::set_verbosity_level(SC_DEBUG);
	
	// do not print, only log. Default is: SC_LOG | SC_DISPLAY
	sc_report_handler::set_actions("tb.dut", SC_INFO, SC_LOG);
	sc_report_handler::set_actions("tb", SC_INFO, SC_LOG);
	
	Fetch_TB * tb = new Fetch_TB("tb", "test_inputs/fetch_ranges.csv", "test_outputs/addresses.log");
	
	SC_REPORT_INFO("main", "simulation starts");
	sc_start(100, SC_NS);
	cout << "Simulation finished after " << sc_time_stamp() << endl;
	delete tb;
	return 0;
}


void Fetch_TB::control_source() {
    rst.write(1);
    wait();
    rst.write(0);
    wait();
    MODULE_INFO("source waiting");
    
    std::string line;
    getline(in, line);  // consume column names
    
    while (getline(in, line)) {
        if (line.empty()) break;
        
        // extract start and end from CSV
        line.replace(line.find(","), 1, " ");
        std::istringstream iss(line);
        pointer_type start, end;
        iss >> start >> end;
        
        start_addr.write(start);
        end_addr.write(end);
        wait();
        dut.job_start.notify();
        
        wait(dut.done.posedge_event());
    }
    MODULE_INFO("source done");
}

void Fetch_TB::memory_source() {
    wait();
    
    while (true) {
        mem_ready = true;
        wait(mem_read);
        
        mem_ready = false;
        pointer_type ptr = dut.mem_read_address;
        tensor_element value = ptr * 3;
        count_type index = ptr * 2;
        wait();
        
        mem_res_value.write(fiber_entry(index, value));
        wait();
        
        mem_done.notify();
    }
}

void Fetch_TB::ixn_sink() {
    wait();
    
    while (true) {
        fiber_entry ent = fiber.read();
        MODULE_INFO("received " << ent);
    }    
}
