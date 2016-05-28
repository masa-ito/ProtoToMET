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

#include <ParallelizationTypeTag/Default.hpp>

#include <DenseLinAlg/MatrixVector.hpp>


namespace DenseLinAlg {

	namespace mpl = boost::mpl;
	namespace proto = boost::proto;

	namespace PTT = ParallelizationTypeTag;



	// Lazy function object for evaluating an element of
	// the resultant vector from the multiplication of
	// matrix and vector objects.
	//
	// An expression like ( matrix * vector )(index) is transformed
	// into the loop for calculating the dot product between
	// the index'th row of the matrix and the vector.
	struct LazyMatVecMult
	{
		Matrix const& m;
		Vector const& v;
		const int mColSz;

		typedef double result_type;
		// typedef mpl::int_<1> proto_arity;

		explicit LazyMatVecMult(Matrix const& mat, Vector const& vec) :
			m( mat), v( vec), mColSz(mat.rowSize()) {}

		LazyMatVecMult( LazyMatVecMult const& lazy) :
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


	// Lazy function object for evaluating an element of
	// the resultant vector from the multiplication of
	// a matrix and a diagonal matrix and a matrix
	struct LazyMatDiagmatMatMult
	{
		Matrix const & pre;
		DiagonalMatrix const & diag;
		Matrix const & post;
		const int sz;

		typedef double result_type;

		explicit LazyMatDiagmatMatMult(Matrix const & pre_,
			DiagonalMatrix const & diag_, Matrix const & post_) :
			pre( pre_), diag( diag_), post( post_), sz( diag_.size())
		{}

		LazyMatDiagmatMatMult( LazyMatDiagmatMatMult const & lazy) :
			pre( lazy.pre), diag( lazy.diag), post( lazy.post), sz( lazy.sz)
		{}

		result_type operator()(int ri, int ci) const
		{
			result_type elm = 0.0;
			for (int k = 0; k < sz; k++)
				elm += pre( ri, k) * diag( k) * post( k, ci);
			return elm;
		}
	};


	// Callable transform object to make the lazy functor
	// a proto exression for lazily evaluationg the multiplication
	// of a matrix and a diagonal matrix and a matrix
	struct MatDiagmatMatMult : proto::callable
	{
		typedef proto::terminal< LazyMatDiagmatMatMult >::type result_type;

		result_type
		operator()( Matrix const& pre, DiagonalMatrix const & diag,
				Matrix const& post) const
		{
			return proto::as_expr( LazyMatDiagmatMatMult(pre, diag, post) );
		}
	};

}




#endif /* DENSELINALG_LAZYEVALUATOR_HPP_ */
