#pragma once
#include <stdexcept>
#include <systemc.h>



/*
    LOGGING

    Wrapper around built-in SystemC logging that makes it easier to print
    values because of dynamic string conversion in stream operators.
    Note that it requires name() to be defined, so this must be called
    from within a module, not e.g. in sc_main.

    Example usage:
    MODULE_INFO("New values are a = " << a << " and b = " << b);
*/
#include <sstream>
#define MODULE_REPORT(report_fn, report_content) \
{ \
    std::stringstream _temp_ss; \
    _temp_ss << sc_time_stamp() << " " << report_content; \
    report_fn(name(), _temp_ss.str().c_str()); \
}

#define MODULE_INFO(content)    MODULE_REPORT(SC_REPORT_INFO,    content)
#define MODULE_WARNING(content) MODULE_REPORT(SC_REPORT_WARNING, content)
#define MODULE_ERROR(content)   MODULE_REPORT(SC_REPORT_ERROR,   content)

// Tensors can have at most MAX_ORDER dimensions
#define MAX_ORDER 10

/*
    MODULE PARAMETERS

    Here are all the constants related to each module's design. This is for
    things that are constant when elaborating, but could in theory have any
    value. Any constraints should be mentioned for each value.
*/

#define INTERSECTION_FIFO_SIZE 32

// Why would you want this smaller than 2?
// Needs to be indexable by pointer_type (32 bits)
#define MEMORY_SIZE 100000

// can't be zero
#define MEMORY_READ_LATENCY 16
#define MEMORY_WRITE_LATENCY 16

// can't be zero lol
#define PE_COUNT 2



/*
    TYPES

    Here are some types that should remain consistent across the design, but
    the actual choice of type isn't too important. For example, we should
    be able to decide at elaboration time whether tensors are made of integer
    or floating-point elements, and what the bit width is.
*/

typedef float tensor_element;

// I want this to be unsigned, but I also want a NULL sort of feature.
//   Currently using -1 in intersection.cpp
typedef sc_int<32> count_type;

// Should definitely be unsigned. Bit width should reflect total memory space.
typedef sc_uint<32> pointer_type;

struct fiber_entry {
    count_type index;
    tensor_element value;

    fiber_entry(count_type index = -1, tensor_element value = 0) :
        index(index), value(value) {}

    fiber_entry& operator= (const fiber_entry &rhs) {
        index = rhs.index;
        value = rhs.value;
        return *this;
    }

    bool operator== (const fiber_entry &rhs) {
        return index == rhs.index && value == rhs.value;
    }

    friend std::ostream & operator<< (std::ostream & os, const fiber_entry & ent);
};
void sc_trace(sc_trace_file *&, const fiber_entry &, std::string &);

struct job {
	pointer_type a_start;
	pointer_type a_end;
	pointer_type b_start;
	pointer_type b_end;
	pointer_type destination;

    friend std::ostream & operator<< (std::ostream & os, const job & j);
};

struct coord {
    count_type idx[MAX_ORDER];
    const count_type order;

    coord(count_type order) : order(order) {
        if (order > MAX_ORDER)
            throw new std::length_error("MAX_ORDER exceeded in coord init");
        for (int i = 0; i < order; i++)
            this->idx[i] = 0;
    }
    coord(count_type * arr, count_type order) : order(order) {
        if (order > MAX_ORDER) {
			cout << "E1" << endl;
            throw new std::length_error("MAX_ORDER exceeded in coord init");
		}
        if (order < 1) {
			cout << "E2" << endl;
            throw new std::length_error("coord can't have order 0");
		}
        for (int i = 0; i < order; i++)
            this->idx[i] = arr[i];
    }

    coord& operator= (const coord &rhs) {
        if (this->order != rhs.order)
            throw new std::out_of_range("coords have different orders");
        for (int i = 0; i < rhs.order; i++)
            this->idx[i] = rhs.idx[i];
        return *this;
    }

    bool operator== (const coord &rhs) const {
        if (this->order != rhs.order)
            return false;
        for (int i = 0; i < rhs.order; i++)
            if (this->idx[i] != rhs.idx[i])
                return false;
        return true;
    }

    count_type & operator[] (int i) { return this->idx[i]; }
    const count_type operator[] (int i) const { return this->idx[i]; }

    bool zero() const {
        for (int i = 0; i < this->order; i++)
            if (this->idx[i])
                return false;
        return true;
    }

    coord concat(const coord &rhs) const {
        coord new_coord = coord(this->order + rhs.order);
        for (int i = 0; i < this->order; i++)
            new_coord[i] = this->idx[i];
        int j = 0;
        for (int i = this->order; i < new_coord.order; i++)
            new_coord[i] = rhs.idx[j++];

        return new_coord;
    }

	coord last_concat(const coord &rhs) const {
		coord new_coord = coord(this->order + rhs.order - 2);
        for (int i = 0; i < this->order-1; i++)
            new_coord[i] = this->idx[i];
        int j = 0;
        for (int i = this->order-1; i < new_coord.order; i++)
            new_coord[i] = rhs.idx[j++];

        return new_coord;
	}

    coord truncate(count_type drop_idx) const {
        coord new_coord = coord(this->order-1);
        int j = 0;
        for (int i = 0; i < this->order; i++) {
            if (i != drop_idx)
                new_coord[j++] = this->idx[i];
        }
        return new_coord;
    }

    // make contraction destination index
    coord contract(const coord &rhs, count_type lhs_contract, count_type rhs_contract) const {
        if (this->idx[lhs_contract] != rhs[rhs_contract])
            throw new std::length_error("contraction orders unequal");
        return this->truncate(lhs_contract).concat(rhs.truncate(rhs_contract));
    }

    // special case when contract index is last for both coords
    // (I expect this to be the default)
    coord last_contract(const coord &rhs) const {
        if (this->idx[this->order-1] != rhs[rhs.order-1])
            throw new std::length_error("contraction orders unequal");
        coord new_coord = coord(this->order + rhs.order - 2);
        int i;
        for (i = 0; i < this->order-1; i++)
            new_coord[i] = this->idx[i];
        int j = 0;
        for (; i < new_coord.order; i++)
            new_coord[i] = rhs.idx[j++];
        return new_coord;
    }

    friend std::ostream & operator<< (std::ostream & os, const coord & c);
};

struct tensor {
    coord shape;
    pointer_type fibers; // straightforward dense n-D array of fiber pointers

    tensor(coord _shape, pointer_type _fibers)
        : shape(_shape)
        , fibers(_fibers)
    {}

    // increment the referenced index. Returns false if it overflows
    bool increment(coord & c) {
        if (this->shape.order != c.order && this->shape.order != c.order+1) {
            throw new std::invalid_argument("cannot increment unrelated coord");
        }
        // increment through all indices or all but last
        for (int i = 0; i < c.order; i++) {
            if (c.idx[i] < this->shape.idx[i]-1) {
                c.idx[i]++;
                break;
            } else {
                c.idx[i] = 0;
                continue;
            }
            /* // more robust to edge cases but takes more gates I think
            if (c.idx[i] != this->shape.idx[i]-1) {
                c.idx[i]++;
            if (c.idx[i] != this->shape.idx[i])
                break; // done incrementing
            else {
                c.idx[i] = 0;
                continue; // increment next digit
            }*/
        }
        return !c.zero();
    }

	pointer_type coord_2_metaptr(const coord & c) {
		if (this->shape.order != c.order) {
			cout << "E3 tensor shape: " << this->shape.order << ", coord chape: " << c.order << endl;
			throw new std::invalid_argument("cannot convert unrelated coord");
		}
		pointer_type res = fibers;
		pointer_type stride = 1;
		for (int i = this->shape.order-2; i >= 0; i--) {
			res += c[i] * stride;
			stride *= this->shape[i];
		}
		return res;
	}
};
void sc_trace(sc_trace_file *&, const tensor &, std::string &);
