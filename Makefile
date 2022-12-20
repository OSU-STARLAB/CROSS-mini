EXE = sim
LIBS = -lsystemc
TOP_SRC = main.cpp types.cpp
FLAGS = -Wall -g

${EXE}: ${TOP_SRC} processing/* memory/*
	g++ ${FLAGS} -o "${EXE}" ${TOP_SRC} processing/*.cpp memory/* ${LIBS}

test: ${EXE}
	@echo ./${EXE}
	@SYSTEMC_DISABLE_COPYRIGHT_MESSAGE=1 ./${EXE}
	@echo
	cat report.log

clean:
	rm -f *.o a.out ${EXE} *.log test_outputs/* report.log

memory_tb.o: memory/memory_tb.cpp memory/memory.h types.h defines.h
	g++ ${FLAGS} -c memory/memory_tb.cpp

memory.o: memory/memory.cpp memory/memory.h types.h defines.h
	g++ ${FLAGS} -c memory/memory.cpp

types.o: types.cpp types.h defines.h
	g++ ${FLAGS} -c types.cpp

mem: memory_tb.o memory.o types.o
	g++ ${FLAGS} memory_tb.o memory.o types.o ${LIBS}
