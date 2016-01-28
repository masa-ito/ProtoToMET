/*
 * Preconditioner.hpp
 *
 *  Created on: 2015/11/10
 *      Author: Masakatsu ITO
 */

#ifndef SPARSELINALG_PRECONDITIONER_HPP_
#define SPARSELINALG_PRECONDITIONER_HPP_

#include <DenseLinAlg/DenseLinAlg.hpp>

namespace DLA = DenseLinAlg;

namespace SparseLinAlg {

	class AbstPreconditioner;


	struct LazyPreconditioner :
			public DLA::LazyVectorMaker< LazyPreconditioner >
	{
		const AbstPreconditioner & precond;
		const DLA::Vector & b;

		explicit
		LazyPreconditioner( const AbstPreconditioner & preconditioner,
							const DLA::Vector & b_) :
					DLA::LazyVectorMaker< LazyPreconditioner>( b_.size()),
					precond( preconditioner) , b( b_) {}

		void assignDataTo_derived(DLA::Vector & lhs) const;

	};

	class AbstPreconditioner
	{
	public :
		explicit AbstPreconditioner() {}
		virtual ~AbstPreconditioner() {}

		LazyPreconditioner
		solve( const DLA::Vector & b) const {
			return LazyPreconditioner( *this, b);
		}

		virtual void solveAndAssign(const DLA::Vector & b,
									DLA::Vector & lhs) const = 0;
	};

	void LazyPreconditioner::assignDataTo_derived(DLA::Vector & lhs) const
	{
		precond.solveAndAssign(b, lhs);
	}


	class DiagonalPreconditioner : public AbstPreconditioner
	{
	private :
		const int sz;
		double *diagInv;

	public :
		template < typename MatType >
		explicit DiagonalPreconditioner(const MatType & mat) :
		sz(mat.rowSize()), diagInv( new double[sz])
		{
			for (int i = 0; i < sz; i++) diagInv[ i] = 1.0 / mat(i, i);
		}

		virtual ~DiagonalPreconditioner()
		{
			delete [] diagInv;
		}

		virtual void solveAndAssign(const DLA::Vector & b,
									DLA::Vector & lhs) const
		{
			for (int i = 0; i < sz; i++) lhs(i) = diagInv[i] * b(i);
		}
	};


};




#endif /* SPARSELINALG_PRECONDITIONER_HPP_ */
