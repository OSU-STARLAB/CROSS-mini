#include <systemc.h>
#include "intersection.h"

/*
	Intersection Testbench
	
	Instantiates an Intersection Unit (intersection.cpp) and feeds it inputs read
	from the fiber_* definition files. Outputs are written to the outputs file.
	
	This module will instantiate the DUT itself, and only requires a clock signal
	to be supplied from outside.
	
	- source_*_filename is a string filename. The testbench will handle opening
	  and closing the file handles, and will exit(1) if it fails.
	- outputs is also a string filename. It will be overwritten.
	- Input file format is newline-separated pairs of space-separated (idx, value)
	  and must not end with newline because my parsing is dumb
*/
SC_MODULE(Intersection_TB) {
	Intersection dut;
	sc_signal<bool> done_a, done_b;
	
	sc_fifo<fiber_entry> fiber_a, fiber_b;
	sc_fifo<tensor_element> results;
	
	void source_a();
	void source_b();
	void sink();
	
	std::ofstream out;
	std::ifstream in_a, in_b;
	
	SC_HAS_PROCESS(Intersection_TB);
	Intersection_TB(sc_module_name name,
		std::string source_a_filename,
		std::string source_b_filename,
		std::string outputs
	) :
		dut("dut"),
		fiber_a(INTERSECTION_FIFO_SIZE),
		fiber_b(INTERSECTION_FIFO_SIZE),
		results(INTERSECTION_FIFO_SIZE),
		clk("clk_sig", 1, SC_NS)
	{
		in_a.open(source_a_filename);
		if (!in_a.is_open()) {
			perror(source_a_filename.c_str());
			exit(1);
		}
		
		in_b.open(source_b_filename);
		if (!in_b.is_open()) {
			perror(source_b_filename.c_str());
			exit(1);
		}
		
		out.open(outputs);
		if (!out.is_open()) {
			perror(outputs.c_str());
			exit(1);
		}
		
		dut.clk(clk);
		dut.rst(rst);
		dut.fiber_a(fiber_a);
		dut.fiber_b(fiber_b);
		dut.results(results);
		dut.done_a(done_a);
		dut.done_b(done_b);
		
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
};
