
all: diagPrecondConGrad_plainC.o \
	diagPrecondConGrad_nonMetaOpenMP.o


INCDIR = -I..
CPP11STD = -std=c++11

diagPrecondConGrad_plainC.o : diagPrecondConGrad_plainC.cpp 
	${CXX} ${CPP11STD} ${INCDIR} -c $<

diagPrecondConGrad_nonMetaOpenMP.o : diagPrecondConGrad_nonMetaOpenMP.cpp
	${CXX} ${CPP11STD} ${INCDIR} -fopenmp -c $<

