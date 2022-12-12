#include "intersection_tb.h"
#include <random>
#include <time.h>

void Intersection_TB::source_a() {
	tensor_element val;
	count_type idx;
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
				if (fiber_a.num_free() > 0) {
					in_a >> idx;
					in_a >> val;
					fiber_entry ent_a = fiber_entry(idx, val);
					MODULE_INFO("read from file ent_a = " << ent_a);
					
					// send to DUT
					fiber_a.write(ent_a);
					MODULE_INFO("sent to module: ent_a = " << ent_a);
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
	count_type idx;
	done_b = false;
	
	// wait for reset pulse
	wait();
	wait();
	
	do {
		switch (rand() % 3) {
			case 0:
				// read value pair from input file A
				if (fiber_b.num_free() > 0) {
					in_b >> idx;
					in_b >> val;
					fiber_entry ent_b = fiber_entry(idx, val);
					MODULE_INFO("read from file ent_b = " << ent_b);
					
					// send to DUT
					fiber_b.write(ent_b);
					MODULE_INFO("sent to module: ent_b = " << ent_b);
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
	output = results.read();
	out << output << endl;
	MODULE_INFO("got output " << output << " at " << sc_time_stamp());
	sc_stop();
}
