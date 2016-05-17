/*
 * Preconditioner.hpp
 *
 *  Created on: 2015/11/10
 *      Author: Masakatsu ITO
 */

#ifndef SPARSELINALG_PRECONDITIONER_HPP_
#define SPARSELINALG_PRECONDITIONER_HPP_

#include <ParallelizationTypeTag/Default.hpp>

#include <DenseLinAlg/DenseLinAlg.hpp>


namespace SparseLinAlg {

	namespace DLA = DenseLinAlg;
	namespace PTT = ParallelizationTypeTag;

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

		template < typename MatType >
		void init( const MatType & mat,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >&)
		{
			for (int i = 0; i < sz; i++) diagInv[ i] = 1.0 / mat(i, i);
		}

		template < typename MatType >
		void init( const MatType & mat,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >&)
		{
			#pragma omp parallel for
			for (int i = 0; i < sz; i++) diagInv[ i] = 1.0 / mat(i, i);
			std::cout << "OpenMP preconditioner init" << std::endl;
		}

		void _solveAndAssign(const DLA::Vector & b, DLA::Vector & lhs,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >&)
		const
		{
			for (int i = 0; i < sz; i++) lhs(i) = diagInv[i] * b(i);
		}

		void _solveAndAssign(const DLA::Vector & b, DLA::Vector & lhs,
			const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >&)
		const
		{
			#pragma omp parallel for
			for (int i = 0; i < sz; i++) lhs(i) = diagInv[i] * b(i);
			std::cout << "OpenMP preconditioner solve" << std::endl;
		}

	public :
		template < typename MatType >
		explicit DiagonalPreconditioner(const MatType & mat) :
		sz(mat.rowSize()), diagInv( new double[sz])
		{
			init( mat, PTT::Specified());
		}

		virtual ~DiagonalPreconditioner()
		{
			delete [] diagInv;
		}

		virtual void solveAndAssign(const DLA::Vector & b,
									DLA::Vector & lhs) const
		{
			_solveAndAssign( b, lhs, PTT::Specified());
		}
	};


};




#endif /* SPARSELINALG_PRECONDITIONER_HPP_ */
