
all:	transformingMatrixVectorMultiplication \
	transformingMatVecMultAndVecSub \
	protoDeepCopyMatrixVectorExpr \
	diagPrecondConjGrad_IntroToCFD_Exam4_3 


INCDIR = -I..
CPP11STD = -std=c++11

transformingMatrixVectorMultiplication : transformingMatrixVectorMultiplication.cpp
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

transformingMatVecMultAndVecSub : transformingMatVecMultAndVecSub.cpp
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

protoDeepCopyMatrixVectorExpr : protoDeepCopyMatrixVectorExpr.cpp
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

diagPrecondConjGrad_IntroToCFD_Exam4_3 : diagPrecondConjGrad_IntroToCFD_Exam4_3.cpp
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

