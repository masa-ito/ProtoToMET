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
	// matrix and vector objects.
	//
	// An expression like ( matrix * vector )(index) is transformed
	// into the loop for calculating the dot product between
	// the index'th row of the matrix and the vector.
	template < typename MultithreadingType >
	struct LazyMatVecMult
	{
		Matrix< MultithreadingType > const& m;
		Vector< MultithreadingType > const& v;
		const int mColSz;

		typedef double result_type;

		explicit LazyMatVecMult(Matrix< MultithreadingType > const& mat,
								Vector< MultithreadingType > const& vec) :
			m( mat), v( vec), mColSz(mat.rowSize()) {}

		LazyMatVecMult( LazyMatVecMult< MultithreadingType > const& lazy) :
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
	template < typename MultithreadingType >
	struct MatVecMult : proto::callable
	{
		typedef
			typename
				proto::terminal< LazyMatVecMult< MultithreadingType > >::type
			result_type;

		result_type
		operator()( Matrix< MultithreadingType > const& mat,
					Vector< MultithreadingType > const& vec) const
		{
			return proto::as_expr(
					LazyMatVecMult< MultithreadingType >(mat, vec) );
		}
	};


	// Lazy function object for evaluating an element of
	// the resultant vector from the multiplication of
	// a matrix and a diagonal matrix and a matrix
	template < typename MultithreadingType >
	struct LazyMatDiagmatMatMult
	{
		Matrix< MultithreadingType > const & pre;
		DiagonalMatrix< MultithreadingType > const & diag;
		Matrix< MultithreadingType > const & post;
		const int sz;

		typedef double result_type;

		explicit LazyMatDiagmatMatMult(
				Matrix< MultithreadingType > const & pre_,
				DiagonalMatrix< MultithreadingType > const & diag_,
				Matrix< MultithreadingType > const & post_) :
			pre( pre_), diag( diag_), post( post_), sz( diag_.size())
		{}

		LazyMatDiagmatMatMult(
			LazyMatDiagmatMatMult< MultithreadingType > const & lazy) :
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
	template < typename MultithreadingType >
	struct MatDiagmatMatMult : proto::callable
	{
		typedef
		typename
		proto::terminal< LazyMatDiagmatMatMult< MultithreadingType > >::type
		result_type;

		result_type
		operator()( Matrix< MultithreadingType > const& pre,
					DiagonalMatrix< MultithreadingType > const & diag,
					Matrix< MultithreadingType > const& post) const
		{
			return
				proto::as_expr(
					LazyMatDiagmatMatMult< MultithreadingType >(pre,
																diag, post)
				);
		}


		/* typedef double result_type;
		template < typename Sig > struct result;

		template < typename This, typename T1, typename T2, typename T3,
					typename T4, typename T5 >
		struct result< This(T1, T2, T3, T4, T5) > { typedef double type; };

		result_type
		operator()( Matrix const& pre, DiagonalMatrix const & diag,
				Matrix const& post, int ri, int ci) const
		{
			result_type elm = 0.0;
			for (int k = 0; k < diag.size(); k++)
				elm += pre( ri, k) * diag( k) * post( k, ci);
			return elm;
		} */
	};

	/*
	struct LazyMatDiagmatMatMult
	{
		typedef double result_type;

		explicit LazyMatDiagmatMatMult() {}
		LazyMatDiagmatMatMult( LazyMatDiagmatMatMult const & lazy) {}

		result_type
		operator()( Matrix const& pre, DiagonalMatrix const & diag,
					Matrix const& post, int ri, int ci) const
		{
			result_type elm = 0.0;
			for (int k = 0; k < diag.size(); k++)
				elm += pre( ri, k) * diag( k) * post( k, ci);
			return elm;
		}
	};
	*/
}




#endif /* DENSELINALG_LAZYEVALUATOR_HPP_ */
