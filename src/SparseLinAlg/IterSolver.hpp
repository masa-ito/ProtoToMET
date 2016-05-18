/*
 * IterSolver.hpp
 *
 *  Created on: 2015/11/10
 *      Author: Masakatsu ITO
 */

#ifndef SPARSELINALG_ITERSOLVER_HPP_
#define SPARSELINALG_ITERSOLVER_HPP_

#include <limits>

#include <DenseLinAlg/DenseLinAlg.hpp>

namespace DLA = DenseLinAlg;

namespace SparseLinAlg {

	struct LazyIterSolver;

	class AbstIterSolver
	{
	public :
		explicit AbstIterSolver() {}
		virtual ~AbstIterSolver() {}

		LazyIterSolver
		solve( const DLA::Vector & b, const DLA::Vector & iniGuess,
			   const double convgergenceCriterion = 1.0e-5,
			   const int maxIter = std::numeric_limits<int>::max()) const;

		virtual void solveAndAssign(const DLA::Vector & b,
					const DLA::Vector & iniGuess,
					DLA::Vector & lhs,
					double convgergenceCriterion,
					int maxIter= std::numeric_limits<int>::max()) const = 0;
	};

	struct LazyIterSolver : public DLA::LazyVectorMaker< LazyIterSolver >
	{
		const AbstIterSolver & solver;
		const DLA::Vector & b, iniGuess;
		const double criterion;
		const int maxIter;

		explicit
		LazyIterSolver( const AbstIterSolver & solver_,
						const DLA::Vector & b_, const DLA::Vector & iniGuess_,
						double convgergenceCriterion = 1.0e-5,
						const int maxIter_ = std::numeric_limits<int>::max()) :
			DLA::LazyVectorMaker< LazyIterSolver >( b_.size() ),
			solver(solver_), b( b_), iniGuess( iniGuess_),
			criterion(convgergenceCriterion), maxIter( maxIter_) {}

		void assignDataTo_derived(DLA::Vector & lhs) const
		{
			solver.solveAndAssign(b, iniGuess, lhs, criterion, maxIter);
		}
	};

	LazyIterSolver
	AbstIterSolver::solve( const DLA::Vector & b,
			const DLA::Vector & iniGuess,
			double convgergenceCriterion,
			const int maxIter) const
	{
		return  LazyIterSolver( *this,
						b, iniGuess, convgergenceCriterion, maxIter);
	}


	template <typename MatType, typename PreType>
	class ConjugateGradient : public AbstIterSolver
	{
	private :
		const MatType & coeff;
		const PreType & precond;

	public :
		explicit ConjugateGradient(const MatType & coefficients,
				const PreType & preconditioner ) :
				coeff( coefficients), precond( preconditioner) {}

		virtual void solveAndAssign( const DLA::Vector & b,
					const DLA::Vector & iniGuess,
					DLA::Vector & lhs,
					const double convgergenceCriterion,
					const int maxIter = std::numeric_limits<int>::max()) const
		{
			DLA::Vector resid( b.columnSize()), z( b.columnSize()),
					q( b.columnSize());

			resid = b - coeff * iniGuess;
			z = precond.solve( resid);
			double rho = resid.dot( z);

			DLA::Vector p = z;
			q = coeff * p;
			double alpha = rho / p.dot(q);

			lhs = iniGuess + alpha * p;
			resid -= alpha * q;

			const double bAbs = b.abs();
			for (int iter = 0;
					iter < maxIter &&
					resid.abs() / bAbs >  convgergenceCriterion ;
					iter++ )
			{
				z = precond.solve( resid);
				double prevRho = rho;
				rho = resid.dot( z);

				double beta = rho / prevRho;
				p = z + beta * p;

				q = coeff * p;
				alpha = rho / p.dot(q);
				lhs += alpha * p;
				resid -= alpha * q;
			}

		}
	};

}



#endif /* SPARSELINALG_ITERSOLVER_HPP_ */
