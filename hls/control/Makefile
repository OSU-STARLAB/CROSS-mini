COMMON_DEPS = ../spec/*
CXX = g++ -Wall -g

OBJDIR = build
TB = tb_control

#$(OBJDIR)/memory.o: memory.cpp memory.h $(COMMON_DEPS)
#	@[ ! -d $(@D) ] & mkdir -p $(@D)
#	$(CXX) -c $< -o $@

$(TB): control_tb.cpp $(COMMON_DEPS)
	$(CXX) -lsystemc -o $@ control_tb.cpp control.cpp ../spec/types.cpp ../memory/build/memory.o ../processing/build/pe.o

.PHONY: clean
clean:
	rm -rf $(OBJDIR)
	rm -f $(TB) a.out *.o
