EXE = sim
LIBS = -lsystemc -lm -lstdc++
TOP_SRC = main.cpp types.cpp

${EXE}: Makefile ${TOP_SRC} processing/*
	g++ -o "${EXE}" ${TOP_SRC} processing/*.cpp ${LIBS}

test: ${EXE}
	./${EXE}
	@echo
	cat report.log

clean:
	rm -f *.o a.out ${EXE} *.log test_outputs/* report.log
