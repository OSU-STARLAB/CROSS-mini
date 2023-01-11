COMMON_DEPS = ../spec/*
CXX = g++ -Wall -g

OBJDIR = build
SRC = fetch.cpp intersection.cpp store.cpp
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))

# By default, build all functionality as non-testbench
$(OBJDIR)/pe.o: pe.cpp pe.h $(OBJ) $(COMMON_DEPS)
	@[ ! -d $(@D) ] & mkdir -p $(@D)
	$(CXX) -c $< -o $(OBJDIR)/pe_tmp.o
	ld -r -o $@ \
		$(OBJDIR)/fetch.o \
		$(OBJDIR)/intersection.o \
		$(OBJDIR)/store.o \
		$(OBJDIR)/pe_tmp.o

# Top-level testbench
pe_tb: pe_tb.cpp pe_tb.h pe.cpp pe.h $(OBJ) $(COMMON_DEPS)
	$(CXX) -lsystemc -o $@ $< pe.cpp $(OBJ) ../spec/types.cpp

# Fetch testbench
fetch_tb: fetch_tb.cpp $(OBJDIR)/fetch.o $(COMMON_DEPS)
	$(CXX) -lsystemc -o $@ $< $(OBJDIR)/fetch.o ../spec/types.cpp

# Intersection testbench
intersection_tb: intersection_tb.cpp $(OBJDIR)/intersection.o $(COMMON_DEPS)
	$(CXX) -lsystemc -o $@ $< $(OBJDIR)/intersection.o ../spec/types.cpp

# Store testbench
store_tb: store_tb.cpp $(OBJDIR)/store.o $(COMMON_DEPS)
	$(CXX) -lsystemc -o $@ $< $(OBJDIR)/store.o ../spec/types.cpp

$(OBJDIR)/fetch.o: fetch.cpp fetch.h $(COMMON_DEPS)
	@[ ! -d $(@D) ] & mkdir -p $(@D)
	$(CXX) -c $< -o $@

$(OBJDIR)/intersection.o: intersection.cpp intersection.h $(COMMON_DEPS)
	@[ ! -d $(@D) ] & mkdir -p $(@D)
	$(CXX) -c $< -o $@

$(OBJDIR)/store.o: store.cpp store.h $(COMMON_DEPS)
	@[ ! -d $(@D) ] & mkdir -p $(@D)
	$(CXX) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJDIR) pe_tb fetch_tb intersection_tb store_tb a.out *.o
