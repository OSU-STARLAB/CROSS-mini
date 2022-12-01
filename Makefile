EXE = sim
LIBS = -lsystemc -lm -lstdc++
SRC = *.cpp

${EXE}: ${SRC} *.h Makefile
	g++ -o "${EXE}" ${SRC} ${LIBS}

test: ${EXE}
	./${EXE} && cat report.log

clean:
	rm -f *.o a.out ${EXE} *.log test_outputs/* report.log
