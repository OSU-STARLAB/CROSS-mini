COMMON_DEPS = types.h defines.h Makefile
LIBS = -lsystemc
CC = g++ -Wall -g
LD_COMBINE = ld -r

# Executables
tb_control: ${COMMON_DEPS} control/control_tb.cpp memory.o pe.o
	${CC} -o tb_control ${LIBS} control/control_tb.cpp memory.o pe.o types.o

tb_mem: ${COMMON_DEPS} main.cpp memory/memory_tb.cpp memory.o types.o
	${CC} -o tb_mem ${LIBS} memory/memory_tb.cpp memory.o types.o

tb_pe: ${COMMON_DEPS} types.o pe.o processing/pe_tb.cpp
	${CC} -o tb_pe ${LIBS} main.cpp processing/pe_tb.cpp types.o pe.o


# Common
types.o: ${COMMON_DEPS} types.cpp
	${CC} -c types.cpp


# Processing Elements
pe.o: ${COMMON_DEPS} processing/pe.cpp processing/pe.h fetch.o intersection.o store.o
	${CC} -c processing/pe.cpp
	${LD_COMBINE} fetch.o intersection.o store.o pe.o -o pe_combined.o
	mv pe_combined.o pe.o

fetch.o: ${COMMON_DEPS} processing/fetch.cpp processing/fetch.h
	${CC} -c processing/fetch.cpp

intersection.o: ${COMMON_DEPS} processing/intersection.cpp processing/intersection.h
	${CC} -c processing/intersection.cpp

store.o: ${COMMON_DEPS} processing/store.cpp processing/store.h
	${CC} -c processing/store.cpp


# Memory
memory.o: ${COMMON_DEPS} memory/memory.cpp memory/memory.h
	${CC} -c memory/memory.cpp


# Utility
.PHONY: clean
clean:
	rm -f *.o a.out tb_control tb_mem tb_pe *.log test_outputs/* report.log

