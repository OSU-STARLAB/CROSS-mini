#include "store.h"

void Store::store_main() {
    wait();

    while (true) {
        mem_write_value = results.read();
        mem_write_address = destination.read();

        do wait(); while (!mem_ready);  // ensure at least one cycle
        MODULE_INFO("notifying");
        mem_write.notify();  // tell mem to do the thing
		MODULE_INFO("between notify and wait");
        wait(mem_done);  // block until mem finishes
        MODULE_INFO("done waiting for store");
    }
}
