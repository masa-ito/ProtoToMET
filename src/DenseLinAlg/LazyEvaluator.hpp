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

	// For OpenMP
	struct LazyMatVecMultOmp
	{
		Matrix const& m;
		Vector const& v;
		const int mColSz;

		typedef double result_type;

		explicit LazyMatVecMultOmp(Matrix const& mat, Vector const& vec) :
			m( mat), v( vec), mColSz(mat.rowSize()) {}

		LazyMatVecMultOmp( LazyMatVecMultOmp const& lazy) :
			m(lazy.m), v(lazy.v), mColSz(lazy.mColSz) {}

		result_type operator()(int index) const
		{
			result_type elm = 0.0;

			#pragma omp parallel for reduction (+:elm)
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

	// For OpenMP
	struct MatVecMultOmp : proto::callable
	{
		typedef proto::terminal< LazyMatVecMultOmp >::type result_type;

		result_type
		operator()( Matrix const& mat, Vector const& vec) const
		{
			return proto::as_expr( LazyMatVecMultOmp(mat, vec) );
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
