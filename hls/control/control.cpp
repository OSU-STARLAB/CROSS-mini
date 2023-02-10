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
	//pointer_type fiber_start = mem.append_fiber(fiber);
	pointer_type fiber_start = 1;
	
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

