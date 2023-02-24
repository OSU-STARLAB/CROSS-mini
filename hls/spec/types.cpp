#include "types.h"
#include <systemc.h>

std::ostream & operator<< (std::ostream & os, const fiber_entry & ent) {
    os << '(' << ent.index << ',' << ent.value << ')';
    return os;
}

void sc_trace(sc_trace_file *& f, const fiber_entry & ent, std::string & name) {
    sc_trace(f, ent.index, name + ".idx");
    sc_trace(f, ent.value, name + ".val");
}

std::ostream & operator<< (std::ostream & os, const coord & c) {
    os << "({" << c.order << "} ";
    for (int i = 0; i < c.order-1; i++)
        os << c[i] << ", ";
    os << c[c.order-1] << ')';
    return os;
}

void sc_trace(sc_trace_file *& f, const coord & c, std::string & name) {
    sc_trace(f, c.order, name+".order");
    std::stringstream _temp_ss;
    for (int i = 0; i < c.order; i++) {
        _temp_ss.clear();
        _temp_ss << name << ".idx[" << c[i] << ']';
        sc_trace(f, c.idx[i], _temp_ss.str().c_str());
    }
}

std::ostream & operator<< (std::ostream & os, const job & j) {
	os << "job <" << hex << &j << dec << ">";
	return os;
}

void sc_trace(sc_trace_file *& f, const job & j, std::string & name) {
    sc_trace(f, j.a_start, name+".a_start");
    sc_trace(f, j.a_end, name+".a_end");
    sc_trace(f, j.b_start, name+".b_start");
    sc_trace(f, j.b_end, name+".b_end");
    sc_trace(f, j.destination, name+".dest");
    std::stringstream _temp_ss;
}
