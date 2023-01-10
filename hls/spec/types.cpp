#include "types.h"

std::ostream & operator<< (std::ostream & os, const fiber_entry & ent) {
    os << '(' << ent.index << ',' << ent.value << ')';
    return os;
}

void sc_trace(sc_trace_file *& f, const fiber_entry & ent, std::string & name) {
    sc_trace(f, ent.index, name + ".idx");
    sc_trace(f, ent.value, name + ".val");
}
