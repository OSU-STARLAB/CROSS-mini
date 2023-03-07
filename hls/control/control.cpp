#include "control.h"

void Control::main () {
	wait(1, SC_NS);

}

pointer_type unpack_pointer(std::ifstream & in) {
	u_int32_t dest;
	in.read(reinterpret_cast<char*>(&dest), 4);
	pointer_type p = dest;
	return p;
}

fiber_entry unpack_fiber_entry(std::ifstream & in) {
	u_int32_t index;
	float value;
	in.read(reinterpret_cast<char*>(&index), 4);
	in.read(reinterpret_cast<char*>(&value), 4);
	fiber_entry f;
	f.index = index;
	f.value = value;
	return f;
}

void Control::contract() {
	pointer_type start_A = tensor_A.read();
	count_type shape_A_arr[MAX_ORDER];
	count_type order_A = (int32_t)metadata[start_A].read();
	wait(1, SC_NS);

	pointer_type start_B = tensor_B.read();
	count_type shape_B_arr[MAX_ORDER];
	count_type order_B = (int32_t)metadata[start_B].read();
	wait(1, SC_NS);

	for (pointer_type i = 0; i < metadata[start_A]; i++) {
		shape_A_arr[i] = metadata[start_A + i].read();
	}
	for (pointer_type i = 0; i < metadata[start_B]; i++) {
		shape_B_arr[i] = metadata[start_B + i].read();
	}
	wait(1, SC_NS);

	cout << "T1 " << shape_A_arr << " " << order_A << endl;
	coord shape_A = coord(shape_A_arr, order_A);
	coord iter_A = coord(order_A);
	tensor A(shape_A, start_A);

	coord shape_B = coord(shape_B_arr, order_B);
	coord iter_B = coord(order_B);
	tensor B(shape_B, start_B);

	coord shape_C = shape_A.last_contract(shape_B);
	tensor C(shape_C, mem.append_idx);
	metadata[append_idx++] = (uint32_t)shape_C.order;
	for (pointer_type i = 0; (long long int)i < shape_C.order; i++) {
		metadata[append_idx++] = (uint32_t)shape_C[i];
	}

	// allocate dense denstination :(
	coord iter_C = coord(shape_C.order);
	bool cont = true;
	while (cont) {
		metadata[append_idx++] = (uint32_t)C.coord_2_metaptr(iter_C);
		cont = C.increment(iter_C);
		wait(1, SC_NS);
	}
	cout << "T6" << endl;

	// TODO: combine above and below loops

	// distribute jobs
	cont = true;
	while (cont) {
		// jobdest = iter_A.concat(iter_B);
		pointer_type a_start, a_end, b_start, b_end, dest;
		cout << "T7a" << endl;
		a_start = metadata[A.coord_2_metaptr(iter_A)];
		cout << "T7b" << endl;
		a_end = metadata[A.coord_2_metaptr(iter_A)+1];
		cout << "T7c" << endl;
		b_start = metadata[B.coord_2_metaptr(iter_B)];
		cout << "T7d" << endl;
		b_end = metadata[B.coord_2_metaptr(iter_B)+1];
		cout << "T7e" << endl;
		dest = (uint32_t)C.coord_2_metaptr(iter_A.concat(iter_B));
		cout << "T7f" << endl;
		jobs.write(job{a_start, a_end, b_start, b_end, dest}); /*
			metadata[A.coord_2_metaptr(iter_A)],
			metadata[A.coord_2_metaptr(iter_A)+1],
			metadata[B.coord_2_metaptr(iter_B)],
			metadata[B.coord_2_metaptr(iter_B)+1],
			(uint32_t)C.coord_2_metaptr(iter_A.concat(iter_B))
		});*/
		cout << "T7g" << endl;
		// TODO: destination pointer is probably wrong. Need to think about shape more

		cont = A.increment(iter_A);
		if (!cont) {
			cont = B.increment(iter_B);
		}
		wait(1, SC_NS);
	}
	contract_done.notify();
}

void Control::PE_done_watch() {
	// If I integrate this directly in the loop below it could cause a problem:
	//  - Multiple PEs finish in the same cycle
	//  - The first one blocks when reading from the jobs fifo
	//  - A new job is generated and added to the fifo so execution continues
	//  - Now the second PE that's ready for a job might not cound as .triggered()?
	// This scenario requires testing since I don't see it in documentation.
	// This might be a situation where I should use an sc_event_queue.
	while (true) {
		wait(PEs_done);
		for (int i = 0; i < PE_COUNT; i++) {
			if (jobs_done[i].triggered())
				PEs_running[i] = false;
		}
	}
}

void Control::distribute_jobs() {
	// skip initialization phase
	int idx = 0;
	wait();

	// round robin. TODO: maybe make this not be round robin
	while (true) {
		if (PEs_running[idx] == true) {
			idx++;
			if (idx >= PE_COUNT)
				idx = 0;
			wait();
			continue;
		}
		job j = jobs.read();
		fiber_a_starts[idx].write(j.a_start);
		fiber_a_ends[idx].write(j.a_end);
		fiber_b_starts[idx].write(j.b_start);
		fiber_b_ends[idx].write(j.b_end);
		destinations[idx].write(j.destination);
		PEs_running[idx] = true;
		wait();
		jobs_start[idx].notify();

		idx++;
		if (idx >= PE_COUNT)
			idx = 0;
		wait();
	}
}

pointer_type Control::append_tensor_file(std::string filename) {
	std::ifstream in;
	in.open(filename, ios::binary);
	if (!in.is_open()) {
		perror(filename.c_str());
		exit(1);
	}
	pointer_type start = append_idx;
	// store tensor order
	pointer_type order = unpack_pointer(in);
	metadata[append_idx++] = order;
	cout << "order is " << order << endl;

	// store tensor shape
	pointer_type shape;
	for (u_int32_t i = 0; i < order; i++) {
		shape = unpack_pointer(in);
		cout << "shape is " << shape << endl;
		metadata[append_idx++] = shape;
	}

	// store tensor fibers in main memory
	int len = unpack_pointer(in);
	std::vector<fiber_entry> fiber;
	for (int i = 0; i < len; i++) {
		fiber.push_back(unpack_fiber_entry(in));
		cout << fiber.back() << " ";
	}
	pointer_type fiber_start = mem.append_fiber(fiber);
	//pointer_type fiber_start = 1;

	// and pointers to those fibers in metadata memory
	len = unpack_pointer(in);
	cout << endl << "len is " << len << endl;
	for (int i = 0; i < len; i++) {
		metadata[append_idx++] = unpack_pointer(in) + fiber_start;
	}
	in.close();
	return start;
}

void Control::print_region(pointer_type start, pointer_type end) {
	cout << "Meta:" << endl;
	for (pointer_type i = start; i <= end; i++) {
		cout << "  [" << i << "] = " << metadata[i] << endl;
	}
}

