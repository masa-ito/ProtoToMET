/*
 * IterSolver.hpp
 *
 *  Created on: 2015/11/10
 *      Author: Masakatsu ITO
 */

#ifndef SPARSELINALG_ITERSOLVER_HPP_
#define SPARSELINALG_ITERSOLVER_HPP_

namespace SparseLinAlg {

	class AbstIterSolver;

	template <typename VecType>
	struct LazyIterSolver
	{
		AbstIterSolver & solver;
		const VecType & b, iniGuess;
		const double criterion;

		explicit LazyIterSolver( AbstIterSolver & solver_,
							const VecType & b_, const VecType & iniGuess_,
							const double convgergenceCriterion = 1.0e-5) :
			solver(solver_), b( b_), iniGuess( iniGuess_),
			criterion(convgergenceCriterion) {}

		void solveAndAssignTo(VecType & lhs) const
		{
			solver.solveAndAssign(b, iniGuess, lhs, criterion);
		}
	};

	class AbstIterSolver
	{
	public :
		explicit AbstIterSolver() {}
		virtual ~AbstIterSolver() {}

		template <typename VecType>
		LazyIterSolver< VecType>
		solve( const VecType & b, const VecType & iniGuess,
				const double convgergenceCriterion = 1.0e-5)
		{
			return  LazyIterSolver< VecType>( *this,
							b, iniGuess, convgergenceCriterion);
		}

		template <typename VecType>
		virtual void solveAndAssign(const VecType& b,
								const VecType & iniGuess,
								VecType & lhs,
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

		template <typename VecType>
		virtual void solveAndAssign( const VecType& b,
								const VecType & iniGuess,
								VecType & lhs,
								const double convgergenceCriterion) const
		{
			VecType resid = b - coeff * iniGuess,
					z = precond.solve( resid);
			double rho = resid.dot( z);

			VecType p = z,
					q = coeff * p;
			double alpha = rho / p.dot(q);

			lhs = iniGuess + alpha * p;
			resid = resid - alpha * q;

			double rhsAbs = b.abs();
			while ( resid.abs() / rhsAbs >  convgergenceCriterion)
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
