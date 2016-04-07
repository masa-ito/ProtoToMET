/*
 * ParallelizationTypeTag.hpp
 *
 *  Created on: 2016/03/23
 *      Author: Masakatsu ITO
 */

#ifndef PARALLELIZATIONTYPETAG_PARALLELIZATIONTYPETAG_HPP_
#define PARALLELIZATIONTYPETAG_PARALLELIZATIONTYPETAG_HPP_

//
// Parallelization Type Tag
//
namespace ParallelizationTypeTag {

	// Multithreading types
	struct OpenMP {};
	struct SingleThread {};

	// Vectoriztion (SIMD extension) type
	// I still am not sure about the difference in using
	// various SIMD extension types.
	/* ??
	struct SSEx {};
	struct AVX {};
	struct Altivec {};
	?? */

	// Multiprocessing types
	struct MPI {};
	struct SingleProcess {};

}


#endif /* PARALLELIZATIONTYPETAG_PARALLELIZATIONTYPETAG_HPP_ */
