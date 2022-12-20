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
    _temp_ss << report_content; \
    report_fn(name(), _temp_ss.str().c_str()); \
}

#define MODULE_INFO(content)    MODULE_REPORT(SC_REPORT_INFO,    content)
#define MODULE_WARNING(content) MODULE_REPORT(SC_REPORT_WARNING, content)
#define MODULE_ERROR(content)   MODULE_REPORT(SC_REPORT_ERROR,   content)



/*
    MODULE PARAMETERS
    
    Here are all the constants related to each module's design. This is for
    things that are constant when elaborating, but could in theory have any
    value. Any constraints should be mentioned for each value.
*/

#define INTERSECTION_FIFO_SIZE 4
#define MEMORY_SIZE 100
#define MEMORY_READ_LATENCY 3
#define MEMORY_WRITE_LATENCY 2
#define PE_COUNT 8
