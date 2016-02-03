/*
 * LazyEvaluator.hpp
 *
 *  Created on: 2015/11/22
 *      Author: Masakatsu ITO
 */

#ifndef DENSELINALG_LAZYEVALUATOR_HPP_
#define DENSELINALG_LAZYEVALUATOR_HPP_


#include <math.h>

#include <iostream>
#include <boost/proto/proto.hpp>

#include <DenseLinAlg/MatrixVector.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


namespace DenseLinAlg {

	// Lazy function object for evaluating an element of
	// the resultant vector from the multiplication of
	// a matrix and vector objects.
	//
	// An expression like ( matrix * vector )(index) is transformed
	// into the loop for calculating the dot product between
	// the index'th row of the matrix and the vector.
	struct LazyMatVecMult
	{
		Matrix const& m;
		Vector const& v;
		int mColSz;

		typedef double result_type;
		// typedef mpl::int_<1> proto_arity;

		explicit LazyMatVecMult(Matrix const& mat, Vector const& vec) :
		m( mat), v( vec), mColSz(mat.rowSize()) {}

		LazyMatVecMult(LazyMatVecMult const& lazy) :
			m(lazy.m), v(lazy.v), mColSz(lazy.mColSz) {}

		result_type operator()(int index) const
		{
			result_type elm = 0.0;
			for (int ci =0;  ci < mColSz; ci++)
				elm += m(index, ci) * v(ci);
			return elm;
		}
	};

	// Callable transform object to make the lazy functor
	// a proto exression for lazily evaluationg the multiplication
	// of a matrix and a vector .
	struct MatVecMult : proto::callable
	{
		typedef proto::terminal< LazyMatVecMult >::type result_type;

		result_type
		operator()( Matrix const& mat, Vector const& vec) const
		{
			return proto::as_expr( LazyMatVecMult(mat, vec) );
		}
	};


//	struct LazyDoubleToVector
//	{
//		const double val;
//		const int sz;
//
//		explicit LazyDoubleToVector(double val_, int size) :
//				val( val_), sz( size) {}
//
//		int rowSize() const { return 1; }
//		int columnSize() const { return sz; }
//		int size() const { return sz; }
//
//		double operator()(int i) const { return val; }
//	};


}




#endif /* DENSELINALG_LAZYEVALUATOR_HPP_ */
