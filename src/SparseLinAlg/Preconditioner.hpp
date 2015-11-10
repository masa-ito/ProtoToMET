/*
 * Preconditioner.hpp
 *
 *  Created on: 2015/11/10
 *      Author: Masakatsu ITO
 */

#ifndef SPARSELINALG_PRECONDITIONER_HPP_
#define SPARSELINALG_PRECONDITIONER_HPP_

namespace SparseLinAlg {

	class AbstPreconditioner;


	template <typename VecType>
	struct LazyPreconditioner
	{
		AbstPreconditioner & precond;
		const VecType & b;

		explicit LazyPreconditioner( AbstPreconditioner & preconditioner,
									const VecType & b_) :
					precond( preconditioner) , b( b_) {}

		void solveAndAssignTo(VecType & lhs) const
		{
			precond.solveAndAssign(b, lhs);
		}
	};

	class AbstPreconditioner
	{
	public :
		explicit AbstPreconditioner() {}
		virtual ~AbstPreconditioner() {}

		template <typename VecType>
		LazyPreconditioner< VecType>
		solve( const VecType & b) {
			return LazyPreconditioner< VecType>( *this, b);
		}

		template <typename VecType>
		virtual void solveAndAssign(const VecType& b,
									VecType & lhs) const = 0;
	};


	template <typename MatType>
	class DiagonalPreconditioner : public AbstPreconditioner
	{
	private :
		const int sz;
		double *diagInv;

	public :
		explicit DiagonalPreconditioner(const MatType & mat) :
		sz(mat.rowSize()), diagInv( new double[sz])
		{
			for (int i = 0; i < sz; i++) diagInv = 1.0 / mat(i, i);
		}

		virtual ~DiagonalPreconditioner()
		{
			delete [] diagInv;
		}

		template <typename VecType>
				virtual void solveAndAssign(const VecType& b,
											VecType & lhs) const
		{
			for (int i = 0; i < sz; i++) lhs(i) = diagInv[i] * b(i);
		}
	};


};




#endif /* SPARSELINALG_PRECONDITIONER_HPP_ */
