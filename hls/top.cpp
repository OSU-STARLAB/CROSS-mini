#include <systemc>
#include "control/control.h"

SC_MODULE(Top) {
    Control control;
	sc_signal<pointer_type> tensor_A;
	sc_signal<pointer_type> tensor_B;
	std::string filename_lhs, filename_rhs, filename_res;

    void top_main();

    SC_CTOR(Top)
		: control("ctrl")
        , clk("clk_sig", 1, SC_NS)
        , rst("rst")
    {
		control.tensor_A(tensor_A);
		control.tensor_B(tensor_B);
        SC_THREAD(top_main);
    }

    private:
        sc_clock clk;
        sc_signal<bool> rst;
};

int sc_main(int argc, char * argv[]) {
	if (argc != 4) {
		cout << endl << "Usage: " << argv[0] <<
			" LHS.csfbin RHS.csfbin RES.csfbin" << endl;
		cout << "Must specify both inputs and result filename." << endl;
		cout << "Inputs are contracted along their final dimensions." << endl;
		return 1;
	}
	Top * top;
	try {
		top = new Top("top");
	} catch (std::exception& e) {
		SC_REPORT_ERROR(1,(std::string(e.what())+" Please fix elaboration errors and retry.").c_str());
	    return 1;
	} catch (...) {
	    SC_REPORT_ERROR(1,"Caught exception during elaboration");
		return 1;
	}
	top->filename_lhs = argv[1];
	top->filename_rhs = argv[2];
	top->filename_res = argv[3];
	try {
	    sc_start(1000000, SC_NS);
	} catch (std::exception& e) {
		SC_REPORT_WARNING(1,(std::string("Caught exception ")+e.what()).c_str());
	} catch (...) {
		SC_REPORT_ERROR(1,"Caught exception during simulation.");
	}
	if (not sc_end_of_simulation_invoked()) {
		cout << "Simulation stopped without explicit sc_stop()" << endl;
		sc_stop();
		cout << "sc_stop() called" << endl;
	}
	double elapsed_ps = sc_time_stamp().to_double(); // default unit is picoseconds
    cout << "Simulation finishes in " << elapsed_ps/1000 << " ns" << endl;
    return 0;
}

void Top::top_main() {
	pointer_type ta = control.append_tensor_file(filename_lhs);
	pointer_type tb = control.append_tensor_file(filename_rhs);
	wait(1, SC_NS);
	tensor_A.write(ta);
	tensor_B.write(tb);
	wait(1, SC_NS);
	control.contract_start.notify();
	wait(control.contract_done);
	cout << "contract_done was notified" << endl;
	pointer_type tc = control.tensor_C.read();
	wait(1, SC_NS);
	control.extract_tensor_file(filename_res, tc);
	control.print_region(0, 50);
	control.mem.print_region(0, 200);

    sc_stop();
}
