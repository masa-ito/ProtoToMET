TARGET=diagPrecondConGrad_plainC.o \
	diagPrecondConGrad_nonMetaOpenMP.o

all: ${TARGET}


INCDIR = -I..
CPP11STD = -std=c++11

diagPrecondConGrad_plainC.o : diagPrecondConGrad_plainC.cpp 
	${CXX} ${CPP11STD} ${INCDIR} -c $<

diagPrecondConGrad_nonMetaOpenMP.o : diagPrecondConGrad_nonMetaOpenMP.cpp
	${CXX} ${CPP11STD} ${INCDIR} -fopenmp -c $<

clean:
	rm ${TARGET} 
