
all:	transformingMatrixVectorMultiplication \
	transformingMatVecMultAndVecSub \
	protoDeepCopyMatrixVectorExpr 


INCDIR = -I..
CPP11STD = -std=c++11

transformingMatrixVectorMultiplication : transformingMatrixVectorMultiplication.cpp
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

transformingMatVecMultAndVecSub : transformingMatVecMultAndVecSub.cpp
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

protoDeepCopyMatrixVectorExpr : protoDeepCopyMatrixVectorExpr.cpp
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

