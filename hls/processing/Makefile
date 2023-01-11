COMMON_DEPS = ../spec/*
CXX = g++ -Wall -g

OBJDIR = build
SRC = fetch.cpp intersection.cpp store.cpp
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))

$(OBJDIR)/pe.o: pe.cpp pe.h $(OBJ) $(COMMON_DEPS)
	@[ ! -d $(@D) ] & mkdir -p $(@D)
	$(CXX) -c $< -o $(OBJDIR)/pe_tmp.o
	ld -r -o $@ \
		$(OBJDIR)/fetch.o \
		$(OBJDIR)/intersection.o \
		$(OBJDIR)/store.o \
		$(OBJDIR)/pe_tmp.o

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
	rm -rf $(OBJDIR) a.out *.o
