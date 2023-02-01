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
