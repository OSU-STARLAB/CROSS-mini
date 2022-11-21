#include "intersection.h"

void Intersection::intersection_main() {
    // initialization
    bool just_read = false;
    tensor_element accumulator = 0;
    count_type idx_a=-1, idx_b=-1;  // fails if count_type is unsigned (which it should be...)
    tensor_element a=0, b=0;
    // this initialization will follow the first code path,
    // but since a and b are 0, the accumulator isn't affected.
    
    wait();  // end initialization
    
    while (true) {
        MODULE_INFO("accumulator is " << accumulator);
        // check if we're done
        if (!just_read) {
            if (done_b &&
                    values_a.num_available() != 0 &&
                    values_b.num_available() == 0) {
                // flush a FIFOs
                idx_a = indices_a.read();
                a = values_a.read();
                MODULE_INFO("flushing a");
                if (idx_a != idx_b) {
                    MODULE_INFO("surprise collision!");
                    continue;
                }
            }
            else if (done_a &&
                    values_a.num_available() == 0 &&
                    values_b.num_available() != 0) {
                // flush b FIFOs
                idx_b = indices_b.read();
                b = values_b.read();
                MODULE_INFO("flushing b");
                if (idx_a != idx_b) {
                    MODULE_INFO("surprise collision!");
                    continue;
                }
            }
            else if (done_a && done_b &&
                    values_a.num_available() == 0 &&
                    values_b.num_available() == 0) {
                // actually done
                MODULE_INFO("emitting");
                results.write(accumulator);
                accumulator = 0;
                
                // trigger "advance both" condition next to re-initialize
                a = 0;
                b = 0;
                idx_a = -1;
                idx_b = -1;
                MODULE_INFO("standing by to re-initialize");
            }
        }
        
        // check for intersection
        if (just_read)
            MODULE_INFO("Just read idx_a = " << idx_a << ", idx_b = " << idx_b);
        just_read = false;
        if (idx_a == idx_b) {
            // combine then advance either a or b
            accumulator += a * b;
            MODULE_INFO("intersection at idx = " << idx_a << " performs "
                << "acc += " << a << " * " << b << "  -->  " << accumulator);
            
            // advance either fiber (there must be an easier way to do this)
            //   (or initialize if just started)
            //   (or re-initialize if just flushed)
            do {
                if (indices_a.nb_read(idx_a)) {
                    values_a.read(a);
                    MODULE_INFO("just received idx_a = " << idx_a << ", a = " << a);
                    just_read = true;
                }
                
                if (indices_b.nb_read(idx_b)) {
                    values_b.read(b);
                    MODULE_INFO("?just received idx_b = " << idx_b << ", b = " << b);
                    just_read = true;
                }

                if (!just_read) {
                    MODULE_INFO("waiting for either fiber");
                    wait();
                }
            } while (!just_read);
            //MODULE_INFO("exited loop");
            
        } else if (idx_a < idx_b) {
            // advance only a
            MODULE_INFO("trying to advance a");
            if (indices_a.nb_read(idx_a)) {
                a = values_a.read();
                MODULE_INFO("just received idx_a = " << idx_a << ", a = " << a);
                just_read = true;
            }
            
        } else if (idx_a > idx_b){
            // advance only b
            MODULE_INFO("trying to advance b");
            if (indices_b.nb_read(idx_b)) {
                b = values_b.read();
                MODULE_INFO("just received idx_b = " << idx_b << ", b = " << b);
                just_read = true;
            }
            
        } else {
            MODULE_WARNING("Unexpected case in intersection unit. "
                << "a = " << a << ", b = " << b
                << ", idx_a = " << idx_a << ", idx_b = " << idx_b
                << ", acc = " << accumulator
            );
        }
        wait();  // This while loop does (best-case) one clock cycle.
        // Because each FIFO read and write could block, this might me optional.
    }
}