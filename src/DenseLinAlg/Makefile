TARGET=diagPrecondConGrad_plainC.o \
	diagPrecondConGrad_nonMetaOpenMP.o

HEADERS= DenseLinAlg.hpp \
 Grammar.hpp \
 MatrixVector.hpp \
 LazyEvaluator.hpp \
 diagPrecondConGrad.hpp

all: ${TARGET}


INCDIR = -I..
CPP11STD = -std=c++11

diagPrecondConGrad_plainC.o : diagPrecondConGrad_plainC.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} -c $<

diagPrecondConGrad_nonMetaOpenMP.o : diagPrecondConGrad_nonMetaOpenMP.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} -fopenmp -c $<

clean:
	rm ${TARGET} 
