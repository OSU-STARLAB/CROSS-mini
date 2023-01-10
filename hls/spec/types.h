#pragma once
#include <systemc.h>
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

    bool operator== (const fiber_entry & rhs) {
        return index == rhs.index && value == rhs.value;
    }
    
    friend std::ostream & operator<< (std::ostream & os, const fiber_entry & ent);    
};
void sc_trace(sc_trace_file *&, const fiber_entry &, std::string &);
