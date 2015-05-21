
all:	helloWorld lazyVector \
	protofyingIntArray3 protofyingStdVector \
	protofyingArrayHeap protofyingArrayWrapper protofyingStdVectorWrapper \
	vectorAdaptingNonProtoTermType adaptingArrayWrapper

helloWorld : helloWorld.cpp
	${CXX} $< -o $@
	
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
