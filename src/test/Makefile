
all:	transformingMatrixVectorMultiplication \
	transformingMatVecMultAndVecSub \
	transformingDiagMatVecMult \
	transformingMatDiagmatMatMult \
	protoDeepCopyMatrixVectorExpr \
	diagPrecondConjGrad_IntroToCFD_Exam4_3 

HEADERS= ../DenseLinAlg/DenseLinAlg.hpp \
 ../DenseLinAlg/Grammar.hpp \
 ../DenseLinAlg/MatrixVector.hpp \
 ../DenseLinAlg/LazyEvaluator.hpp \
 ../DenseLinAlg/diagPrecondConGrad.hpp 


INCDIR = -I..
CPP11STD = -std=c++11

transformingMatrixVectorMultiplication :  \
 transformingMatrixVectorMultiplication.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

transformingMatVecMultAndVecSub : transformingMatVecMultAndVecSub.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

transformingDiagMatVecMult : transformingDiagMatVecMult.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

transformingMatDiagmatMatMult : transformingMatDiagmatMatMult.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

protoDeepCopyMatrixVectorExpr : protoDeepCopyMatrixVectorExpr.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

diagPrecondConjGrad_IntroToCFD_Exam4_3 : \
 diagPrecondConjGrad_IntroToCFD_Exam4_3.cpp \
 ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

