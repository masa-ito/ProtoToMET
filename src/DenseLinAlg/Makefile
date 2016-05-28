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
OPTIMIZATION= -O3 -Winline \
 --param max-inline-recursive-depth=32 \
 --param max-inline-insns-single=2000


diagPrecondConGrad_plainC.o : diagPrecondConGrad_plainC.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} ${OPTIMIZATION} -c $<

diagPrecondConGrad_nonMetaOpenMP.o : diagPrecondConGrad_nonMetaOpenMP.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} ${OPTIMIZATION} -fopenmp -c $<

clean:
	rm ${TARGET} 
