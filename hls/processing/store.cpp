#include "store.h"

void Store::store_main() {
    wait();

    while (true) {
        mem_write_value = results.read();
        mem_write_address = destinations.read();

        do wait(); while (!mem_ready);  // ensure at least one cycle
        MODULE_INFO("notifying memory to write " << mem_write_value << " at " << mem_write_address);
        mem_write.notify(1, SC_NS);  // tell mem to do the thing
		MODULE_INFO("between notify and wait");
        wait(mem_done);  // block until mem finishes
        MODULE_INFO("done waiting for store");
    }
}
