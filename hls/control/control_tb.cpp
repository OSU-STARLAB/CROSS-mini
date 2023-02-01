#include <systemc>
#include "control.h"
//#include "../memory/memory.h"
//#include "../processing/pe.h"

SC_MODULE(Control_TB) {
    //Control control;
    
    void tb_main();
    
    SC_CTOR(Control_TB)
        : clk("clk_sig", 1, SC_NS)
        , rst("rst")
    {
        SC_THREAD(tb_main);
    }
    
    private:
        sc_clock clk;
        sc_signal<bool> rst;
};

int sc_main(int argc, char * argv[]) {
    Control_TB * tb = new Control_TB("tb");
    sc_start(500, SC_NS);
    cout << "Simulation finished after " << sc_time_stamp() << endl;
    delete tb;
    return 0;
}

void Control_TB::tb_main() {
    count_type shape_arr[3] = {5, 4, 2};
    coord shape = coord(shape_arr, 3);
    tensor A = tensor(shape, 0);
    
    count_type shape_arr2[4] = {3, 2, 3, 2};
    coord shape2 = coord(shape_arr2, 4);
    tensor B = tensor(shape2, 0);

    coord iterA = coord(2);
    coord iterB = coord(3);
    
    cout << "starting coord: " << iterA << " " << iterB << endl;
    cout << "A shape: " << A.shape << endl;
    cout << "B shape: " << B.shape << endl;
    
    bool cont = true;
    while (cont) {
        cout << iterA << iterB << endl;
        cont = A.increment(iterA);
        if (!cont) {
            cout << "--------------" << endl;
            cont = B.increment(iterB);
        }
        wait(1, SC_NS);
    }
    
    sc_stop();
}