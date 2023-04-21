#include "intersection.h"

void Intersection::intersection_main() {
    // initialization
    bool just_read = false;
    tensor_element accumulator = 0;
    fiber_entry ent_a = fiber_entry(-1, 0), ent_b = fiber_entry(-1, 0);
    // this initialization will follow the first code path,
    // but since a and b are 0, the accumulator isn't affected.

    wait();  // end initialization

    while (true) {
        MODULE_INFO("accumulator is " << accumulator << " just_read is " << just_read);
        // check if we're done
        if (!just_read) {
            if (done_b &&
                    fiber_a.num_available() != 0 &&
                    fiber_b.num_available() == 0) {
                // flush a FIFOs
                ent_a = fiber_a.read();
                MODULE_INFO("flushing a");
                if (ent_a.index == ent_b.index) {
                    MODULE_INFO("surprise collision!");
					just_read = true;
                    continue;
                }
            }
            else if (done_a &&
                    fiber_a.num_available() == 0 &&
                    fiber_b.num_available() != 0) {
                // flush b FIFOs
                ent_a = fiber_a.read();
                MODULE_INFO("flushing b");
                if (ent_a.index == ent_b.index) {
                    MODULE_INFO("surprise collision!");
					just_read = true;
                    continue;
                }
            }
            else if (done_a && done_b &&
                    fiber_a.num_available() == 0 &&
                    fiber_b.num_available() == 0) {
                // actually done
                MODULE_INFO("emitting");
                results.write(accumulator);
                accumulator = 0;

                // trigger "advance both" condition next to re-initialize
                ent_a = fiber_entry(-1, 0);
                ent_b = fiber_entry(-1, 0);
                MODULE_INFO("standing by to re-initialize");
            }
        }

        // check for intersection
        if (just_read)
            MODULE_INFO("Just read idx_a = " << ent_a.index << ", idx_b = " << ent_b.index);
        if (ent_a.index == ent_b.index) {
            // combine then advance either a or b
            accumulator += ent_a.value * ent_b.value;
            MODULE_INFO("intersection at idx = " << ent_a.index << " performs "
                << "acc += " << ent_a.value << " * " << ent_b.value
                << "  -->  " << accumulator);

            if (just_read && done_a && done_b &&
                    fiber_a.num_available() == 0 &&
                    fiber_b.num_available() == 0) {
                just_read = false;
                continue;
            }
            just_read = false;
            /*if (just_read) {
                just_read = false;
                wait();
                continue;
            }*/

            // advance either fiber (there must be an easier way to do this)
            //   (or initialize if just started)
            //   (or re-initialize if just flushed)
            do {
                if (fiber_a.nb_read(ent_a)) {
                    MODULE_INFO("just received ent_a = " << ent_a);
                    just_read = true;
                }

                if (fiber_b.nb_read(ent_b)) {
                    MODULE_INFO("?just received ent_b = " << ent_b);
                    just_read = true;
                }

                if (!just_read) {
                    //MODULE_INFO("waiting for either fiber");
                    wait();
                }
                /*
                if (done_a && done_b &&
                        fiber_a.num_available() == 0 &&
                        fiber_b.num_available() == 0) {
                    MODULE_INFO("both fetchers done");
                    break;
                }*/
            } while (!just_read);
            //MODULE_INFO("exited loop");

        } else if (ent_a.index < ent_b.index) {
            // advance only a
            MODULE_INFO("trying to advance a");
            if (fiber_a.nb_read(ent_a)) {
                MODULE_INFO("just received ent_a = " << ent_a);
                just_read = true;
            }
            if (done_a) {
                MODULE_INFO("...but it's done");
                just_read = false;
            }

        } else if (ent_a.index > ent_b.index){
            // advance only b
            MODULE_INFO("trying to advance b");
            if (fiber_b.nb_read(ent_b)) {
                MODULE_INFO("just received ent_b = " << ent_b);
                just_read = true;
            }
            if (done_b) {
                MODULE_INFO("...but it's done");
                just_read = false;
            }

        } else {
            MODULE_WARNING("Unexpected case in intersection unit. "
                << "ent_a = " << ent_a << ", ent_b = " << ent_b
                << ", acc = " << accumulator
            );
        }
        wait();  // This while loop does (best-case) one clock cycle.
        // Because each FIFO read and write could block, this might me optional.
    }
}
