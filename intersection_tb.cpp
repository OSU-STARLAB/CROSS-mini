#include "intersection_tb.h"
#include <random>
#include <time.h>

void Intersection_TB::source_a() {
	tensor_element val;
	done_a = false;
	srand(time(NULL));
	
	// generate reset pulse
	rst.write(1);
	wait();
	rst.write(0);
	wait();
	
	do {
		switch (rand() % 3) {
			case 0:
				// read value pair from input file A
				if (dut.values_a.num_free() > 0) {
					in_a >> idx_a;
					in_a >> val;
					MODULE_INFO("read from file idx_a = " << idx_a << " and a = " << val);
					
					// send to DUT
					dut.values_a.write(val);
					dut.indices_a.write(idx_a);
					MODULE_INFO("sent to module: idx_a = " << idx_a << " and a = " << val);
				}
				break;
		 	case 1:
				wait(2);
			case 2:
				break;
		}	
		wait(1);
	} while (in_a.peek() != EOF);
	
	wait();
	done_a = true;
	MODULE_INFO("source_a done")
}

void Intersection_TB::source_b() {
	tensor_element val;
	done_b = false;
	
	// wait for reset pulse
	wait();
	wait();
	
	do {
		switch (rand() % 3) {
			case 0:
				// read value pair from input file A
				if (dut.values_b.num_free() > 0) {
					in_b >> idx_b;
					in_b >> val;
					MODULE_INFO("read from file idx_b = " << idx_b << " and b = " << val);
					
					// send to DUT
					dut.values_b.write(val);
					dut.indices_b.write(idx_b);
					MODULE_INFO("sent to module: idx_a = " << idx_a << " and a = " << val);
				}
				break;
		 	case 1:
				wait(2);
			case 2:
				break;
		}	
		wait(1);
	} while (in_b.peek() != EOF);
	
	wait();
	done_b = true;
	MODULE_INFO("source_b done")
}

void Intersection_TB::sink() {
	tensor_element output;
	output = dut.results.read();
	out << output << endl;
	MODULE_INFO("got output " << output << " at " << sc_time_stamp());
	sc_stop();
}
