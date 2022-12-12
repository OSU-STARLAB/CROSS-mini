#include "store.h"

void Store::store_main() {
    wait();
    
    while (true) {
        mem_write_value = results.read();
        mem_write_address = destination.read();
        
        do wait(); while (!mem_ready);
        mem_write_start->notify();
        wait(mem_done);
    }
}