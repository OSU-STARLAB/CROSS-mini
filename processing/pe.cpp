#include "pe.h"

void PE::pe_destination_fifo() {
    while (true) {
        wait(job_start);
        wait();
        destinations.write(destination);
    }
}