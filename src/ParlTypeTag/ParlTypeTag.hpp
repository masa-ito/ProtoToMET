/*
 * ParlTypeTag.hpp
 *
 *  Created on: 2016/03/23
 *      Author: Masakatsu ITO
 */

#ifndef PARLTYPETAG_PARLTYPETAG_HPP_
#define PARLTYPETAG_PARLTYPETAG_HPP_

//
// Parallelization Type Tag
//
namespace ParlTypeTag {

	// Multithreading type
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

	// Multiprocessing type
	struct MPI {};
	struct SingleProcess {};

}


#endif /* PARLTYPETAG_PARLTYPETAG_HPP_ */
