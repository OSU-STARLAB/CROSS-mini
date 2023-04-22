#include "control.h"

void Control::main () {
	wait(1, SC_NS);

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

	for (pointer_type i = 0; i < (pointer_type)order_A; i++) {
		shape_A_arr[i] = metadata[start_A + i + 1].read();
	}
	for (pointer_type i = 0; i < (pointer_type)order_B; i++) {
		shape_B_arr[i] = metadata[start_B + i + 1].read();
	}
	wait(1, SC_NS);

	coord shape_A = coord(shape_A_arr, order_A);
	coord iter_A = coord(order_A-1);
	tensor A(shape_A, start_A);

	coord shape_B = coord(shape_B_arr, order_B);
	coord iter_B = coord(order_B-1);
	tensor B(shape_B, start_B);

	coord shape_C = shape_A.last_contract(shape_B);
	tensor C(shape_C, append_idx);
	tensor_C.write(append_idx);  // set module output to metadata pointer for C
	metadata[append_idx++] = (uint32_t)shape_C.order;
	for (pointer_type i = 0; (long long int)i < shape_C.order; i++) {
		metadata[append_idx++] = (uint32_t)shape_C[i];
	}

	// allocate dense denstination :(
	// TODO: allocate in the correct space in the main tensor storage memory
	coord iter_C = coord(shape_C.order);
	coord iter_C_fibers = coord(shape_C.order-1);
	pointer_type fiber_start;
	count_type C_fiber_len = shape_C[shape_C.order-1];
	bool cont = true;
	while (cont) {
		// Choose last coordinate as fiber direction, allocate those.
		// This avoids appending a 1 to the destination shape, but requires forcing
		// a direction but it's fine because it's dense so reshaping is easy
		fiber_start = mem.allocate_fiber(C_fiber_len);
		metadata[append_idx++] = fiber_start;
		cont = C.increment(iter_C_fibers);
		wait(1, SC_NS);
	}
	metadata[append_idx++] = fiber_start + C_fiber_len;  // end of last fiber
	wait(1, SC_NS);  // wait for that last write to actually happen
	//print_region(0, 20);

	// TODO: combine above and below loops? Iteration order seems different maybe

	// distribute jobs
	MODULE_INFO("A tensor is shape " << A.shape << " with start " << A.fibers);
	MODULE_INFO("B tensor is shape " << B.shape << " with start " << B.fibers);
	MODULE_INFO("C tensor is shape " << C.shape << " with start " << C.fibers);
	cont = true;
	while (cont) {
		pointer_type a_start, a_end, b_start, b_end, dest;
		a_start = metadata[A.coord_2_metaptr(iter_A, true)];
		a_end   = metadata[A.coord_2_metaptr(iter_A, true)+1];
		b_start = metadata[B.coord_2_metaptr(iter_B, true)];
		b_end   = metadata[B.coord_2_metaptr(iter_B, true)+1];
		auto c_coord = iter_A.concat(iter_B);
		dest = metadata[C.coord_2_metaptr(c_coord.truncate(c_coord.order-1), true)];
		dest += c_coord[c_coord.order-1];
		count_type dest_idx = c_coord[c_coord.order-1];
		MODULE_INFO("writing job with"
			<< " a_start " << a_start
			<< " a_end "   << a_end
			<< " b_start " << b_start
			<< " b_end "   << b_end
			<< " c_coord " << c_coord
			<< " dest "    << dest
			<< " dest_idx "<< dest_idx
		);
		jobs.write(job{a_start, a_end, b_start, b_end, dest, dest_idx});

		MODULE_INFO("incrementing from A:" << iter_A << " B:" << iter_B << "...");
		cont = B.increment(iter_B);
		if (!cont) {
			cont = A.increment(iter_A);
			cout << "===" << endl;
		}
		MODULE_INFO("   ...to  A:" << iter_A << " B:" << iter_B);
		wait(1, SC_NS);
	}
	MODULE_INFO("Done generating jobs. Waiting for PEs to take them");
	// wait until all PEs finish
	while (jobs.num_available() != 0) wait(1, SC_NS);

	wait(MEMORY_READ_LATENCY, SC_NS);
	MODULE_INFO("job queue empty. Waiting for PEs to finish")
	for (int i = 0; i < PE_COUNT; i++) {
        pes[i]->result_indices.write(-1);
    }
	for (int i = 0; i < PE_COUNT; i++) {
		//while (!pes[i]->done.read())
		while (pes[i]->running.read())
		//while (!(pes[i]->done.read() && !pes[i]->running.read()))
			wait(1, SC_NS);
		MODULE_INFO("PE " << i << " done");
	}/*
    wait(1, SC_NS);
	for (int i = 0; i < PE_COUNT; i++) {
		while (pes[i]->running.read() != false)
			wait(1, SC_NS);
		MODULE_INFO("PE " << i << " done");
	}*/
	MODULE_INFO("all PEs seem to be done");
	contract_done.notify();
}

void Control::PE_done_watch() {
	// If I integrate this directly in the loop below it could cause a problem:
	//  - Multiple PEs finish in the same cycle
	//  - The first one blocks when reading from the jobs fifo
	//  - A new job is generated and added to the fifo so execution continues
	//  - Now the second PE that's ready for a job might not cound as .triggered()?
	// This scenario requires testing since I don't see it in documentation.
	// This might be a situation where I should use an sc_event_queue?
	/*
	while (true) {
		wait(PEs_done);
		for (int i = 0; i < PE_COUNT; i++) {
			if (jobs_done[i].triggered() && PEs_running[i])
				PEs_running[i] = false;
		}
	}
	*//*
	// round robin assign new jobs as PEs finish
	while (true) {
		wait(PEs_done);
		for (int i = 0; i < PE_COUNT; i++) {
			if (jobs_done[i].triggered()) {
				job j = jobs.read(); // blocking here could cause an issue if
									 // multiple PEs finish in the same tick
				fiber_a_starts[i].write(j.a_start);
				fiber_a_ends[i].write(j.a_end);
				fiber_b_starts[i].write(j.b_start);
				fiber_b_ends[i].write(j.b_end);
				destinations[i].write(j.destination);
				jobs_start[i].notify(1, SC_NS);
			}
		}
	}*/
}

void Control::distribute_jobs() {
	// skip initialization phase
	wait(1, SC_NS);

	// first, distribute all available jobs
	for (int i = 0; i < PE_COUNT; i++) {
		job j = jobs.read();
		fiber_a_starts[i].write(j.a_start);
		fiber_a_ends[i].write(j.a_end);
		fiber_b_starts[i].write(j.b_start);
		fiber_b_ends[i].write(j.b_end);
		pes[i]->destinations.write(j.destination);
		pes[i]->result_indices.write(j.dest_idx);
		jobs_start[i].notify(2, SC_NS);
		MODULE_INFO("Job sent to pe"<<i<<" with dest "<<j.destination);
		wait(1, SC_NS);
	}

	// round robin. TODO: maybe make this not be round robin
	while (true) {
		wait(PEs_done);
		for (int i = 0; i < PE_COUNT; i++) {
			if (jobs_done[i].triggered() || !pes[i]->running) {
				job j = jobs.read();
				fiber_a_starts[i].write(j.a_start);
				fiber_a_ends[i].write(j.a_end);
				fiber_b_starts[i].write(j.b_start);
				fiber_b_ends[i].write(j.b_end);
				pes[i]->destinations.write(j.destination);
				pes[i]->result_indices.write(j.dest_idx);
				jobs_start[i].notify();
				MODULE_INFO("Job sent to pe"<<i<<" with dest "<<j.destination);
				wait(1, SC_NS);
			}
		}
	}
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
	cout << "Appending tensor from file " << filename << endl;
	cout << "  order is " << order << endl;

	// store tensor shape
	pointer_type shape;
	for (u_int32_t i = 0; i < order; i++) {
		shape = unpack_pointer(in);
		cout << "  shape is " << shape << endl;
		metadata[append_idx++] = shape;
	}

	// store tensor fibers in main memory
	int len = unpack_pointer(in);
	std::vector<fiber_entry> fiber;
	for (int i = 0; i < len; i++) {
		fiber.push_back(unpack_fiber_entry(in));
		//cout << fiber.back() << " ";
	}
	pointer_type fiber_start = mem.append_fiber(fiber);
	//pointer_type fiber_start = 1;

	// and pointers to those fibers in metadata memory
	len = unpack_pointer(in);
	cout << endl << "  len is " << len << endl;
	for (int i = 0; i < len; i++) {
		metadata[append_idx++] = unpack_pointer(in) + fiber_start;
	}
	in.close();
	return start;
}

void pack_pointer(std::ofstream & out, pointer_type value) {
	u_int32_t val = value;
	out.write(reinterpret_cast<char*>(&val), 4);
}

void pack_fiber_entry(std::ofstream & out, fiber_entry values) {
	u_int32_t idx = values.index;
	float val = values.value;
	if (values.index == -1)
		val = 0;
	out.write(reinterpret_cast<char*>(&idx), 4);
	out.write(reinterpret_cast<char*>(&val), 4);
}

void Control::extract_tensor_file(std::string filename, pointer_type metadata_start) {
	std::ofstream out;
	out.open(filename, ios::binary);
	if (!out.is_open()) {
		perror(filename.c_str());
		exit(1);
	}
	auto idx = metadata_start;
	pointer_type order = metadata[idx++];
	pointer_type shape_arr[MAX_ORDER];
	pack_pointer(out, order);
	// Step through shape counting expected fiber pointers.
	// Also save the shape to the file
	// Since output is dense, tensor volume is entry count
	// (this block would need to change to support sparse results)
	// (I expect that change to not be very difficult)
	int32_t entry_count = 1;
	for (u_int32_t i = 0; i < order; i++) {
		shape_arr[i] = metadata[idx++];
		pack_pointer(out, shape_arr[i]);

		if (i != order-1)
			entry_count *= shape_arr[i];
	}
	int32_t fiber_ptr_count = entry_count + 1;
	entry_count *= shape_arr[order-1];
	// number of entries total
	pack_pointer(out, entry_count);

	cout <<"ptr cnt " << fiber_ptr_count;
	cout << " entries " << entry_count << endl;

	// actual entries, while saving fiber boundries
	std::vector<pointer_type> fiber_bounds;
	pointer_type offset = metadata[idx];
	for (int32_t i = 0; i < fiber_ptr_count-1; i++) {
		pointer_type start = metadata[idx++];
		pointer_type end = metadata[idx];
		fiber_bounds.push_back(start-offset);
		for (u_int32_t j = start; j < end; j++) {
			pack_fiber_entry(out, mem.contents[j]);
		}
	}
	fiber_bounds.push_back(pointer_type(metadata[idx])-offset);

	// number of fiber boundary pointers
	cout << "fiber_bounds.size() = " << fiber_bounds.size() << endl;
	pack_pointer(out, pointer_type(fiber_bounds.size()));

	// actual fiber boundary pointers
	for (auto ptr : fiber_bounds) {
		pack_pointer(out, ptr);
	}
	out.close();
}

void Control::print_region(pointer_type start, pointer_type end) {
	cout << "Meta:" << endl;
	for (pointer_type i = start; i <= end; i++) {
		cout << "  [" << i << "] = " << metadata[i] << endl;
	}
}

