COMMON_DEPS = spec/*
CXX = g++ -Wall -g

OBJDIR=build
SRC=fetch.cpp intersection.cpp store.cpp
OBJ=$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))

# Executables
tb_control: control/control_tb.cpp $(COMMON_DEPS) $(OBJDIR)/types.o $(OBJDIR)/memory.o $(OBJDIR)/pe.o
	$(CXX) -lsystemc -o $@ control/control_tb.cpp $(OBJDIR)/memory.o $(OBJDIR)/pe.o $(OBJDIR)/types.o

#tb_mem: main.cpp memory/memory_tb.cpp $(COMMON_DEPS) $(OBJDIR)/types.o $(OBJDIR)/memory.o
#	$(CXX) -lsystemc -o $@ memory/memory_tb.cpp $(OBJDIR)/memory.o types.o

#tb_pe: processing/pe_tb.cpp $(COMMON_DEPS) types.o pe.o
#	$(CXX) -lsystemc -o $@ main.cpp processing/pe_tb.cpp types.o pe.o

# Common
$(OBJDIR)/types.o: spec/types.cpp $(COMMON_DEPS)
	@[ ! -d $(@D) ] & mkdir -p $(@D)
	$(CXX) -c $< -o $@

# Processing Elements
.PHONY: processing/$(OBJDIR)/pe.o
processing/$(OBJDIR)/pe.o:
	$(MAKE) -C processing
$(OBJDIR)/pe.o: processing/$(OBJDIR)/pe.o
	@[ ! -d $(@D) ] & mkdir -p $(@D)
	cp -u $< $@

# Memory
.PHONY: memory/$(OBJDIR)/memory.o
memory/$(OBJDIR)/memory.o:
	$(MAKE) -C memory
$(OBJDIR)/memory.o: memory/$(OBJDIR)/memory.o
	@[ ! -d $(@D) ] & mkdir -p $(@D)
	cp -u $< $@


# Utility
.PHONY: clean
clean:
	rm -rf $(OBJDIR) 
	rm -f a.out *.o tb_control tb_mem tb_pe
	$(MAKE) -C processing clean
	$(MAKE) -C memory clean
