#include "store.h"

void Store::store_main() {
    wait();
    
    while (true) {
        mem_write_value = results.read();
        mem_write_address = destination.read();
        
        do wait(); while (!mem_ready);  // ensure at least one cycle
        mem_write_start->notify();  // tell mem to pay attention
        wait(mem_done);  // block until mem finishes
    }
}