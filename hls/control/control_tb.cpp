#include <systemc>
#include "control.h"
//#include "../memory/memory.h"
//#include "../processing/pe.h"

SC_MODULE(Control_TB) {
    Control control;
	sc_signal<pointer_type> tensor_A;
	sc_signal<pointer_type> tensor_B;

    void tb_main();
	pointer_type append_tensor_file(std::string filename);

    SC_CTOR(Control_TB)
		: control("DUT")
        , clk("clk_sig", 1, SC_NS)
        , rst("rst")
    {
		control.tensor_A(tensor_A);
		control.tensor_B(tensor_B);
        SC_THREAD(tb_main);
    }

    private:
        sc_clock clk;
        sc_signal<bool> rst;
};

int sc_main(int argc, char * argv[]) {
    Control_TB * tb = new Control_TB("tb");
    sc_start(200, SC_NS);
    cout << "Simulation finished after " << sc_time_stamp() << endl;
    delete tb;
    return 0;
}

pointer_type Control_TB::append_tensor_file(std::string filename) {
	return control.append_tensor_file(filename);
}

void Control_TB::tb_main() {
	/*
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
    cout << "C shape: " << A.shape.last_contract(B.shape) << endl;

    bool cont = true;
    while (cont) {
        cout << "inputs: " << iterA << iterB;
        cout << "\t\toutput: " << iterA.concat(iterB) << endl;
        cont = A.increment(iterA);
        if (!cont) {
            cout << endl;
            cont = B.increment(iterB);
        }
        wait(1, SC_NS);
    }
	cout << "finished incrementing" << endl;
	*/

	pointer_type ta = control.append_tensor_file("../test_inputs/fiber_ax2.csfbin");
	pointer_type tb = control.append_tensor_file("../test_inputs/fiber_ax2.csfbin");
	wait(1, SC_NS);
	tensor_A.write(ta);
	tensor_B.write(tb);
	wait(1, SC_NS);
	control.contract_start.notify();
	wait(control.contract_done);
	cout << "contract_done was notified" << endl;
	pointer_type tc = control.tensor_C.read();
	wait(1, SC_NS);
	control.extract_tensor_file("../test_outputs/out.csfbin", tc);
	control.print_region(0, 20);
	control.mem.print_region(0, 50);

    sc_stop();
}
