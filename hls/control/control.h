#include <systemc.h>
#include "../spec/spec.h"
#include "../memory/memory.h"
#include "../processing/pe.h"

SC_MODULE(Control) {
    Mem mem;
    std::vector<PE*> pes;

    void main();

	// simulation-only helpers
	// returns pointer to start of tensor metadata
	pointer_type append_tensor_file(std::string filename);
	void extract_tensor_file(std::string filename, pointer_type metadata_tensor_start);
	void print_region(pointer_type start, pointer_type end);

	// input pointers, output pointer
	sc_in<pointer_type> tensor_A, tensor_B;
	sc_signal<pointer_type> tensor_C;
	sc_event contract_start;
	sc_event contract_done;

	void contract();
	void distribute_jobs();
	void PE_done_watch();

    SC_CTOR(Control)
        : mem("mem")
        , clk("clk_sig", 1, SC_NS)
        , rst("rst")
        , jobs_start("jobs_start", PE_COUNT)
        , jobs_done("jobs_done", PE_COUNT)
        , fiber_a_starts("fa_starts", PE_COUNT)
        , fiber_a_ends("fa_ends", PE_COUNT)
        , fiber_b_starts("fb_starts", PE_COUNT)
        , fiber_b_ends("fb_ends", PE_COUNT)
        , destinations("destinations", PE_COUNT)
		, jobs("jobs", 64)  // TODO: break out into #define
		, metadata("metadata", 1024)
    {
        mem.clk(clk);
		append_idx = 0;

        for (int i = 0; i < PE_COUNT; i++) {
			PEs_running[i] = false;
            std::string name = "pe";
            name.append(std::to_string(i));
            PE * new_pe = new PE(name.c_str(),
                jobs_start[i], jobs_done[i],  // need job events
                mem.mem_read[i], mem.mem_read_done[i],
                mem.mem_read[i+PE_COUNT], mem.mem_read_done[i+PE_COUNT],
                mem.mem_write[i], mem.mem_write_done[i]
            );
            pes.push_back(new_pe);
            new_pe->clk(clk);
            new_pe->rst(rst);

            new_pe->fiber_a_start(fiber_a_starts[i]);
            new_pe->fiber_a_end(fiber_a_ends[i]);
            new_pe->fiber_b_start(fiber_b_starts[i]);
            new_pe->fiber_b_end(fiber_b_ends[i]);
            new_pe->destination(destinations[i]);

            new_pe->mem_ready(mem.ready);

            mem.read_addr[i](new_pe->mem_read_address_a);

            mem.read_addr[i+PE_COUNT](new_pe->mem_read_address_b);

            new_pe->mem_res_value_a(mem.read_value[i]);
            new_pe->mem_res_value_b(mem.read_value[i+PE_COUNT]);

            mem.write_addr[i](new_pe->mem_write_address_c);
            mem.write_value[i](new_pe->mem_write_value_c);
        }

        for (int i = 0; i < PE_COUNT; i++)
            PEs_done |= jobs_done[i];

        SC_THREAD(main);
		SC_THREAD(contract);
		sensitive << contract_start;
		dont_initialize();
		SC_THREAD(distribute_jobs);
		sensitive << clk;
		SC_THREAD(PE_done_watch);
    }

    ~Control() {
        // not totally sure if this will destruct correctly
        pes.clear();
    }

    private:
        sc_clock clk;
        sc_signal<bool> rst;
        sc_vector<sc_event> jobs_start;
        sc_vector<sc_event> jobs_done;
        sc_vector<sc_signal<pointer_type>> fiber_a_starts;
        sc_vector<sc_signal<pointer_type>> fiber_a_ends;
        sc_vector<sc_signal<pointer_type>> fiber_b_starts;
        sc_vector<sc_signal<pointer_type>> fiber_b_ends;
        sc_vector<sc_signal<pointer_type>> destinations;

		sc_fifo<job> jobs;
		bool PEs_running[PE_COUNT];

		sc_event_or_list PEs_done;

		sc_vector<sc_signal<pointer_type>> metadata;  // tensor pointers
		pointer_type append_idx;
};

