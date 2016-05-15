/*
 * ParallelizationTypeTag.hpp
 *
 *  Created on: 2016/05/15
 *      Author: Masakatsu ITO
 */

#ifndef PARALLELIZATIONTYPETAG_PARALLELIZATIONTYPETAG_HPP_
#define PARALLELIZATIONTYPETAG_PARALLELIZATIONTYPETAG_HPP_

//
// Parallelization Type Tag
//
namespace ParallelizationTypeTag {

	// Vectoriztion (SIMD extension) type hierarchy
	struct NoSIMD {};
	struct SSE : NoSIMD {};
	struct SSE2 : SSE {};
	struct SSE3 : SSE2 {};
	struct SSE4a : SSE3 {};
	struct SSE4_1 : SSE3 {};
	struct SSE4_2 : SSE4_1 {};
	struct AVX : SSE4_2 {};
	// ?? Altivec ???


	// Multithreading types
	template < class SimdTag > struct OpenMP : SimdTag {};
	template < class SimdTag > class SingleThread : SimdTag {};


	// Multiprocessing types
	template < class Multithreding > struct MPI : Multithreding {};
	template < class Multithreding > struct SingleProcess : Multithreding {};

}




#endif /* PARALLELIZATIONTYPETAG_PARALLELIZATIONTYPETAG_HPP_ */
