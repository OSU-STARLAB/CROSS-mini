#pragma once
#include <stdexcept>
#include <systemc.h>
#include "spec.h" // hmmm cyclical. Probably fine...
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

struct coord {
    count_type idx[MAX_ORDER];
    const count_type order;

    coord(count_type order) : order(order) {
        if (order > MAX_ORDER)
            throw new std::out_of_range("MAX_ORDER exceeded in coord init");
    }
    coord(count_type * arr, count_type order) : order(order) {
        if (order > MAX_ORDER)
            throw new std::out_of_range("MAX_ORDER exceeded in coord init");
        if (order < 1)
            throw new std::out_of_range("coord can't havve order 0");
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
    
    count_type operator[] (int i) { return this->idx[i]; }
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
        return this->truncate(lhs_contract).concat(rhs.truncate(rhs_contract));
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
        
    bool increment(coord & c) {
        if (this->shape.order == c.order || this->shape.order == c.order+1) {
            // increment through all indices or all but last
            for (int i = 0; i < this->shape.order; i++) {
                if (this->shape.idx[i] < c.idx[i])
                    this->shape.idx[i]++;
                if (this->shape.idx[i] < c.idx[i])
                    break; // done incrementing
                else {
                    this->shape.idx[i] = 0;
                    continue; // increment next digit
                }
            }
            return c.zero();
        } else {
            throw new std::out_of_range("cannot increment unrelated coord");
        }
    }
};
void sc_trace(sc_trace_file *&, const tensor &, std::string &);
