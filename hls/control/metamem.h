#include <systemc.h>
#include "../spec/spec.h"

// memory for tensor metadata: mostly fiber pointers
#define METASIZE 1024

SC_MODULE(MetaMem) {
	void main();

	SC_CTOR(MetaMem)
		: clk("clk_sig", 1, SC_NS)
		, rst("rst")
		, contents(METASIZE)
	{
		SC_THREAD(main);
		reset_signal_is(rst, true);
		sensitive << clk.posedge_event();
	}
	private:
		sc_clock clk;
		sc_signal<bool> rst;
		std::vector<pointer_type> contents;
};

