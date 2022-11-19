#include <iostream>
#include <string>
#include <systemc.h>
#include "intersection.h"

/*
	Intersection Testbench
	
	Instantiates an Intersection Unit (intersection.cpp) and feeds it inputs read
	from the fiber_a and fiber_b files. Outputs are written to the outputs file.
	
	This module will instantiate the DUT itself, and only requires a clock signal
	to be supplied from outside.
	
	- fiber_* is a string filename. The testbench will handle opening and closing
	  the file handles, and will exit(1) if it fails.
	- outputs is also a string filename. It will be overwritten.
	- Input file format is newline-separated pairs of space-separated (idx, value)
	  and must not end with newline because my parsing is dumb
*/
SC_MODULE(Intersection_TB) {
	Intersection dut;
	sc_in<bool> dut_waiting_on;
	
	void source_a();
	void source_b();
	void sink();
	
	std::ofstream out;
	std::ifstream in_a;
	std::ifstream in_b;
	
	SC_HAS_PROCESS(Intersection_TB);
	Intersection_TB(sc_module_name name,
		std::string fiber_a,
		std::string fiber_b,
		std::string outputs
	) :
		clk("clk_sig", 1, SC_NS),
		dut("dut"),
		done(false),
		idx_a(-1),
		idx_b(-1)
	{
		in_a.open(fiber_a);
		if (!in_a.is_open()) {
			perror(fiber_a.c_str());
			exit(1);
		}
		
		in_b.open(fiber_b);
		if (!in_b.is_open()) {
			perror(fiber_b.c_str());
			exit(1);
		}
		
		out.open(outputs);
		if (!out.is_open()) {
			perror(outputs.c_str());
			exit(1);
		}
		
		dut.clk(clk);
		dut.rst(rst);
		dut_waiting_on(dut.waiting_on);	
		
		SC_THREAD(source_a);
		sensitive << clk.posedge_event();
		
		SC_THREAD(source_b);
		sensitive << clk.posedge_event();
				
		SC_THREAD(sink);
		sensitive << clk.posedge_event();
	}
	
	~Intersection_TB() {
		out.close();
		in_a.close();
		in_b.close();
		MODULE_INFO("files closed");
	}
	
	private:
		sc_clock clk;
		sc_signal<bool> rst;
		bool done;
		tensor_element idx_a, idx_b;
};
