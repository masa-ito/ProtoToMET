
all:	helloWorld lazyVector \
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
	adaptingMyMatrixAndVectorClassesToProto \
	transformingMatrixVectorMultiplication \
	transformingMatVecMultAndVecSub


CPP11STD = -std=c++11

helloWorld : helloWorld.cpp
	${CXX} $< -o $@
#	 ${CXX} helloWorld.cpp -o helloWorld	
	
lazyVector : lazyVector.cpp
	${CXX} $< -o $@

protofyingIntArray3 : protofyingIntArray3.cpp
	${CXX} $< -o $@

protofyingStdVector : protofyingStdVector.cpp
	${CXX} $< -o $@

protofyingArrayWrapper : protofyingArrayWrapper.cpp
	${CXX} $< -o $@

protofyingStdVectorWrapper : protofyingStdVectorWrapper.cpp
	${CXX} $< -o $@

vectorAdaptingNonProtoTermType : vectorAdaptingNonProtoTermType.cpp
	${CXX} $< -o $@

adaptingArrayWrapper : adaptingArrayWrapper.cpp
	${CXX} $< -o $@

adaptingMatrixWithAutoArray2x2 : adaptingMatrixWithAutoArray2x2.cpp
	${CXX} $< -o $@

adaptingMatrixWithHeapArray : adaptingMatrixWithHeapArray.cpp
	${CXX} $< -o $@

adaptingVectorWithHeapArray : adaptingVectorWithHeapArray.cpp
	${CXX} $< -o $@

linAlgAdd : linAlgAdd.cpp
	${CXX} $< -o $@

# linAlgResid : linAlgResid.cpp
# 	${CXX} $< -o $@

transformingVectorOnHeapArray : transformingVectorOnHeapArray.cpp
	${CXX} ${CPP11STD} $< -o $@

transformingVectorOnHeapArrayUsingFunctionType : transformingVectorOnHeapArrayUsingFunctionType.cpp
	${CXX} ${CPP11STD} $< -o $@

transformingMatrixWithHeapArray : transformingMatrixWithHeapArray.cpp
	${CXX} ${CPP11STD} $< -o $@

transformingMatrixAndVector : transformingMatrixAndVector.cpp
	${CXX} ${CPP11STD} $< -o $@

adaptingMyMatrixAndVectorClassesToProto : adaptingMyMatrixAndVectorClassesToProto.cpp
	${CXX} ${CPP11STD} $< -o $@

transformingMatrixVectorMultiplication : transformingMatrixVectorMultiplication.cpp
	${CXX} ${CPP11STD} $< -o $@

transformingMatVecMultAndVecSub : transformingMatVecMultAndVecSub.cpp
	${CXX} ${CPP11STD} $< -o $@

