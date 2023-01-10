COMMON_DEPS = ../spec/*
CXX = g++ -Wall -g

OBJDIR = build

$(OBJDIR)/memory.o: memory.cpp memory.h $(COMMON_DEPS)
	@[ ! -d $(@D) ] & mkdir -p $(@D)
	$(CXX) -c $< -o $@

tb_memory: memory_tb.cpp $(OBJDIR)/memory.o $(COMMON_DEPS)
	$(CXX) -lsystemc -o $@ memory_tb.cpp $(OBJDIR)/memory.o ../spec/types.cpp

.PHONY: clean
clean:
	rm -rf $(OBJDIR)
