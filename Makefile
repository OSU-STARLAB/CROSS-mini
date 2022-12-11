EXE = sim
LIBS = -lsystemc -lm -lstdc++

${EXE}: Makefile main.cpp processing/*
	g++ -o "${EXE}" main.cpp processing/*.cpp ${LIBS}

test: ${EXE}
	./${EXE} && cat report.log

clean:
	rm -f *.o a.out ${EXE} *.log test_outputs/* report.log
