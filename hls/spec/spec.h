#pragma once
#include "types.h"

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

// can't be zero?
#define PE_COUNT 64
