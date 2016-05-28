
all:	transformingMatrixVectorMultiplication \
	transformingMatVecMultAndVecSub \
	transformingMatVecMultAndVecSub_metaOpenMP \
	transformingDiagMatVecMult \
	transformingMatDiagmatMatMult \
	protoDeepCopyMatrixVectorExpr \
	diagPrecondConjGrad_IntroToCFD_Exam4_3 \
	diagPrecondConjGrad_IntroToCFD_Exam4_3_metaOpenMP \
	diagPrecondConjGrad_IntroToCFD_Exam4_3_OneIteration \
	diagPrecondConjGrad_IntroToCFD_Exam4_3_OneIteration_metaOpenMP \
	diagPrecondConjGrad_IntroToCFD_Exam4_3_plainC \
	diagPrecondConjGrad_IntroToCFD_Exam4_3_nonMetaOpenMP

DLA_HEADERS= ../DenseLinAlg/DenseLinAlg.hpp \
 ../DenseLinAlg/Grammar.hpp \
 ../DenseLinAlg/MatrixVector.hpp \
 ../DenseLinAlg/LazyEvaluator.hpp \
 ../DenseLinAlg/diagPrecondConGrad.hpp 

SLA_HEADERS= ../SparseLinAlg/SparseLinAlg.hpp \
 ../SparseLinAlg/IterSolver.hpp \
 ../SparseLinAlg/Preconditioner.hpp 

INCDIR= -I..
CPP11STD= -std=c++11
OPTIMIZATION= -O3 -Winline \
 --param max-inline-recursive-depth=32 \
 --param max-inline-insns-single=2000


transformingMatrixVectorMultiplication :  \
 transformingMatrixVectorMultiplication.cpp \
 ${DLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

transformingMatVecMultAndVecSub : transformingMatVecMultAndVecSub.cpp \
 ${DLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

transformingMatVecMultAndVecSub_metaOpenMP : \
 transformingMatVecMultAndVecSub.cpp \
 ${DLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} -fopenmp $< -o $@


transformingDiagMatVecMult : transformingDiagMatVecMult.cpp \
 ${DLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

transformingMatDiagmatMatMult : transformingMatDiagmatMatMult.cpp \
 ${DLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

protoDeepCopyMatrixVectorExpr : protoDeepCopyMatrixVectorExpr.cpp \
 ${DLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

diagPrecondConjGrad_IntroToCFD_Exam4_3 : \
 diagPrecondConjGrad_IntroToCFD_Exam4_3.cpp airCooledCylinder.hpp \
 ${DLA_HEADERS} ${SLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} ${OPTIMIZATION} $< -o $@

diagPrecondConjGrad_IntroToCFD_Exam4_3_metaOpenMP : \
 diagPrecondConjGrad_IntroToCFD_Exam4_3.cpp airCooledCylinder.hpp \
 ${DLA_HEADERS} ${SLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} ${OPTIMIZATION} -fopenmp $< -o $@	

diagPrecondConjGrad_IntroToCFD_Exam4_3_OneIteration : \
 diagPrecondConjGrad_IntroToCFD_Exam4_3_OneIteration.cpp \
 ${DLA_HEADERS} ${SLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

diagPrecondConjGrad_IntroToCFD_Exam4_3_OneIteration_metaOpenMP : \
 diagPrecondConjGrad_IntroToCFD_Exam4_3_OneIteration.cpp \
 ${DLA_HEADERS} ${SLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} -fopenmp $< -o $@

diagPrecondConjGrad_IntroToCFD_Exam4_3_plainC : \
 diagPrecondConjGrad_IntroToCFD_Exam4_3_plainC.cpp \
 ../DenseLinAlg/diagPrecondConGrad_plainC.o \
 ${DLA_HEADERS} ${SLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} ${OPTIMIZATION} $< \
	../DenseLinAlg/diagPrecondConGrad_plainC.o -o $@

diagPrecondConjGrad_IntroToCFD_Exam4_3_nonMetaOpenMP : \
 diagPrecondConjGrad_IntroToCFD_Exam4_3_plainC.cpp \
 ../DenseLinAlg/diagPrecondConGrad_nonMetaOpenMP.o \
 ${DLA_HEADERS} ${SLA_HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} ${OPTIMIZATION} -fopenmp $< \
	../DenseLinAlg/diagPrecondConGrad_nonMetaOpenMP.o -o $@

