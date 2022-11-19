#include "intersection_tb.h"
#include <random>
#include <time.h>

void Intersection_TB::source_a() {
	tensor_element val;
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
				if (dut.values_a.num_free() > 0 && in_a.peek() != EOF) {
					in_a >> idx_a;
					in_a >> val;
					MODULE_INFO("read from file idx_a = " << idx_a << " and a = " << val);
					
					// send to DUT
					dut.values_a.write(val);
					dut.indices_a.write(idx_a);
				}
				break;
		 	case 1:
				wait(2);
			case 2:
				wait(1);
		}	
	} while (in_a.peek() != EOF);
	
	wait();
	wait();
	if (dut_waiting_on == 1) {  // if waiting on me, say not to
		MODULE_INFO("source_a gonna notify b/c waiting on me");
		dut.flush.notify();
		MODULE_INFO("source_a notify sent");
	} else if (done) {
		MODULE_INFO("source_a gonna notify b/c both done");
		dut.flush.notify();
		MODULE_INFO("source_a notify sent");
	} else
		MODULE_INFO("source_a not notifying")
	done = true;
}

void Intersection_TB::source_b() {
	tensor_element val;
	
	// wait for reset pulse
	wait();
	wait();
	
	do {
		switch (rand() % 3) {
			case 0:
				// read value pair from input file A
				if (dut.values_b.num_free() > 0 && in_b.peek() != EOF) {
					in_b >> idx_b;
					in_b >> val;
					MODULE_INFO("read from file idx_b = " << idx_b << " and b = " << val);
					
					// send to DUT
					dut.values_b.write(val);
					dut.indices_b.write(idx_b);
				}
				break;
		 	case 1:
				wait(2);
			case 2:
				wait(1);
		}	
	} while (in_b.peek() != EOF);
	
	wait();
	wait();
	if (dut_waiting_on == 0) {  // if waiting on me, say not to
		MODULE_INFO("source_b gonna notify b/c waiting on me");
		dut.flush.notify();
		MODULE_INFO("source_b notify sent");
	} else if (done) {
		MODULE_INFO("source_b gonna notify b/c both done");
		dut.flush.notify();
		MODULE_INFO("source_b notify sent");
	} else
		MODULE_INFO("source_b not notifying")
	done = true;
}

void Intersection_TB::sink() {
	tensor_element output;
	output = dut.results.read();
	out << output << endl;
	MODULE_INFO("got output " << output << " at " << sc_time_stamp());
	sc_stop();
}
