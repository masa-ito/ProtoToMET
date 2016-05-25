
HEADERS = ParallelizationTypeTag/ParallelizationTypeTag.hpp \
ParallelizationTypeTag/Default.hpp \
ParallelizationTypeTag/OpenMP.hpp 

EXECUTABLES_FOLLOWING_SUFFIX_RULE= \
helloWorld lazyVector \
protofyingIntArray3 protofyingStdVector \
protofyingArrayWrapper protofyingStdVectorWrapper \
vectorAdaptingNonProtoTermType adaptingArrayWrapper \
adaptingMatrixWithAutoArray2x2 adaptingMatrixWithHeapArray \
adaptingVectorWithHeapArray  \
linAlgAdd \
transformingVectorOnHeapArray \
transformingVectorOnHeapArrayUsingFunctionType \
transformingMatrixWithHeapArray \
transformingMatrixAndVector \
adaptingMyMatrixAndVectorClassesToProto 

TARGET= ${EXECUTABLES_FOLLOWING_SUFFIX_RULE} \
vecAdd \
vecAddSingleThread \
vecAddMetaOpenMP \
matVecMultAndVecSubSingleThread \
matVecMultAndVecSubMetaOpenMP 

all: ${TARGET}

INCDIR = -I.
CPP11STD = -std=c++11


.SUFFIXES: .cpp
.cpp:
	${CXX} ${CPP11STD} $< -o $@

vecAdd : vecAdd.cpp vecAdd.hpp
	${CXX} ${CPP11STD} $< -o $@
	
vecAddSingleThread : vecAddMetaOpenMP.cpp vecAddMetaOpenMP.hpp ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

vecAddMetaOpenMP : vecAddMetaOpenMP.cpp vecAddMetaOpenMP.hpp ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} -fopenmp $< -o $@

matVecMultAndVecSubSingleThread : matVecMultAndVecSubSingleThread.cpp \
 matVecMultAndVecSubMetaOpenMP.hpp ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} $< -o $@

matVecMultAndVecSubMetaOpenMP : matVecMultAndVecSubMetaOpenMP.cpp \
 matVecMultAndVecSubMetaOpenMP.hpp ${HEADERS}
	${CXX} ${CPP11STD} ${INCDIR} -fopenmp $< -o $@
 

#helloWorld : helloWorld.cpp
#	${CXX} $< -o $@
#	
#lazyVector : lazyVector.cpp
#	${CXX} $< -o $@
#
#protofyingIntArray3 : protofyingIntArray3.cpp
#	${CXX} $< -o $@
#
#protofyingStdVector : protofyingStdVector.cpp
#	${CXX} $< -o $@
#
#protofyingArrayWrapper : protofyingArrayWrapper.cpp
#	${CXX} $< -o $@
#
#protofyingStdVectorWrapper : protofyingStdVectorWrapper.cpp
#	${CXX} $< -o $@
#
#vectorAdaptingNonProtoTermType : vectorAdaptingNonProtoTermType.cpp
#	${CXX} $< -o $@
#
#adaptingArrayWrapper : adaptingArrayWrapper.cpp
#	${CXX} $< -o $@
#
#adaptingMatrixWithAutoArray2x2 : adaptingMatrixWithAutoArray2x2.cpp
#	${CXX} $< -o $@
#
#adaptingMatrixWithHeapArray : adaptingMatrixWithHeapArray.cpp
#	${CXX} $< -o $@
#
#adaptingVectorWithHeapArray : adaptingVectorWithHeapArray.cpp
#	${CXX} $< -o $@
#
#linAlgAdd : linAlgAdd.cpp
#	${CXX} $< -o $@
#
#transformingVectorOnHeapArray : transformingVectorOnHeapArray.cpp
#	${CXX} ${CPP11STD} $< -o $@
#
#transformingVectorOnHeapArrayUsingFunctionType : transformingVectorOnHeapArrayUsingFunctionType.cpp
#	${CXX} ${CPP11STD} $< -o $@
#
#transformingMatrixWithHeapArray : transformingMatrixWithHeapArray.cpp
#	${CXX} ${CPP11STD} $< -o $@
#
#transformingMatrixAndVector : transformingMatrixAndVector.cpp
#	${CXX} ${CPP11STD} $< -o $@
#
#adaptingMyMatrixAndVectorClassesToProto : adaptingMyMatrixAndVectorClassesToProto.cpp
#	${CXX} ${CPP11STD} $< -o $@




