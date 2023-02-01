COMMON_DEPS = ../spec/*
CXX = g++ -Wall -g

OBJDIR = build

#$(OBJDIR)/memory.o: memory.cpp memory.h $(COMMON_DEPS)
#	@[ ! -d $(@D) ] & mkdir -p $(@D)
#	$(CXX) -c $< -o $@

tb_control: control_tb.cpp $(COMMON_DEPS)
	$(CXX) -lsystemc -o $@ control_tb.cpp ../spec/types.cpp

.PHONY: clean
clean:
	rm -rf $(OBJDIR)
	rm -f a.out *.o
