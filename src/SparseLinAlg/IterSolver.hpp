/*
 * IterSolver.hpp
 *
 *  Created on: 2015/11/10
 *      Author: Masakatsu ITO
 */

#ifndef SPARSELINALG_ITERSOLVER_HPP_
#define SPARSELINALG_ITERSOLVER_HPP_

#include <DenseLinAlg/DenseLinAlg.hpp>

namespace DLA = DenseLinAlg;

namespace SparseLinAlg {

	class AbstIterSolver;

	struct LazyIterSolver : DLA::LazyVectorMaker
	{
		AbstIterSolver & solver;
		DLA::Vector & b, iniGuess;
		const double criterion;

		explicit
		LazyIterSolver( AbstIterSolver & solver_,
						const DLA::Vector & b_, const DLA::Vector & iniGuess_,
						const double convgergenceCriterion = 1.0e-5) :
			DLA::LazyVectorMaker( b_.size() ),
			solver(solver_), b( b_), iniGuess( iniGuess_),
			criterion(convgergenceCriterion) {}

		void assignDataTo(DLA::Vector & lhs) const
		{
			solver.solveAndAssign(b, iniGuess, lhs, criterion);
		}
	};

	class AbstIterSolver
	{
	public :
		explicit AbstIterSolver() {}
		virtual ~AbstIterSolver() {}

		LazyIterSolver
		solve( const DLA::Vector & b, const DLA::Vector & iniGuess,
				const double convgergenceCriterion = 1.0e-5)
		{
			return  LazyIterSolver( *this,
							b, iniGuess, convgergenceCriterion);
		}

		virtual void solveAndAssign(const DLA::Vector & b,
								const DLA::Vector & iniGuess,
								DLA::Vector & lhs,
								const double convgergenceCriterion) const = 0;
	};


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
								const double convgergenceCriterion) const
		{
			DLA::Vector resid = b - coeff * iniGuess,
					z = precond.solve( resid);
			double rho = resid.dot( z);

			DLA::Vector p = z,
					q = coeff * p;
			double alpha = rho / p.dot(q);

			lhs = iniGuess + alpha * p;
			resid = resid - alpha * q;

			const double bAbs = b.abs();
			while ( resid.abs() / bAbs >  convgergenceCriterion)
			{
				z = precond.solve( resid);
				double prevRho = rho;
				rho = resid.dot( z);

				double beta = rho / prevRho;
				p = z + beta * p;

				q = coeff * p;
				alpha = rho / p.dot(q);
				lhs = lhs + alpha * p;
				resid = resid - alpha * q;
			}

		}
	};

}





#endif /* SPARSELINALG_ITERSOLVER_HPP_ */
