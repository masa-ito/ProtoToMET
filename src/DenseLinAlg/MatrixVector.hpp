/*
 * MatrixVector.hpp
 *
 *  Created on: 2015/10/20
 *      Author: Masakatsu ITO
 */

#ifndef DENSELINALG_MATRIXVECTOR_HPP_
#define DENSELINALG_MATRIXVECTOR_HPP_

#include <math.h>

#include <iostream>
#include <boost/proto/proto.hpp>

#include <ParallelizationTypeTag/Default.hpp>

#include <DenseLinAlg/Grammar.hpp>


namespace DenseLinAlg {

	namespace mpl = boost::mpl;
	namespace proto = boost::proto;

	namespace PTT = ParallelizationTypeTag;


	// A wrapper for a linear algebraic expression
	template < typename E > struct ExprWrapper;

	// The above grammar is associated with this domain.
	struct Domain
		: proto::domain<proto::generator<ExprWrapper>, ExprGrammar>
	{};

	// A wrapper template for linear algebraic expressions
	// including matrices and vectors
	template<typename ExprType>
	struct ExprWrapper
		: proto::extends<ExprType, ExprWrapper<ExprType>, Domain>
	{
		// const int rowSz, colSz;

		explicit ExprWrapper(const ExprType& e)
			: proto::extends<ExprType, ExprWrapper<ExprType>, Domain>(e) /*,
			  rowSz( e.rowSize()), colSz( e.columnSize()) */
		{}

		// int rowSize() const { return rowSz; }
		// int columnSize() const { return colSz; }
	};


	class Vector;

	template < typename Derived >
	struct LazyVectorMaker
	{
		const int sz;

		explicit LazyVectorMaker( int size) : sz( size) {}
		virtual ~LazyVectorMaker() {}

		int rowSize() const { return 1; }
		int columnSize() const { return sz; }
		int size() const { return sz; }

		void assignDataTo(Vector& lhs) const {
			static_cast< const Derived & >( *this).assignDataTo_derived( lhs);
		}
	};


	class Matrix;

	void diagPrecondConGrad( Vector & ansVec,
			const Matrix & coeffMat, const Vector & rhsVec,
			const Vector & initGuessVec,
			double convergenceCriterion);

	// These classes are to be used as the template parameter
	// for AssignType.
	struct AssignFunctor {  // operator=()
		void operator()( double& lhs, double rhs) const { lhs = rhs; }
	};
	struct PlusAssignFunctor {  // operator+=()
		void operator()( double& lhs, double rhs) const { lhs += rhs; }
	};
	struct MinusAssignFunctor {  // operator-=()
		void operator()( double& lhs, double rhs) const { lhs -= rhs; }
	};

	template < typename AssignType > struct AssignVecExpr;

	// Function object for lazily assigning
	// an vector object (not expression temaplte) into a vector object
	template < typename AssignType >
	struct AssignVector {
		void operator()(
			const Vector& rhs, Vector& lhs,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& )
		const;

		void operator()(
			const Vector& rhs, Vector& lhs,
			const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& )
		const;
	};


	class Vector {
	private:
		const int sz;
		double* data;

		double _dot( const Vector& vec,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >&)
		const
		{
			double d = data[0] * vec.data[0];
			for (int i = 1; i < sz; i++) d += data[i] * vec.data[i];
			return d;
		}

		double _dot( const Vector& vec,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >&)
		const
		{
			double d = data[0] * vec.data[0];
			#pragma omp parallel for reduction (+:d)
			for (int i = 1; i < sz; i++) d += data[i] * vec.data[i];

			// std::cout << "OpenMP dot product" << std::endl;

			return d;
		}

		double _abs(
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >&)
		const {
			double aSqr = data[0] * data[0];
			for (int i = 1; i < sz; i++) aSqr += data[i] * data[i];
			return sqrt( aSqr);
		}

		double _abs(
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >&)
		const {
			double aSqr = data[0] * data[0];
			#pragma omp parallel for reduction (+:aSqr)
			for (int i = 1; i < sz; i++) aSqr += data[i] * data[i];

			// std::cout << "OpenMP vector abs" << std::endl;

			return sqrt( aSqr);
		}


	public:
		template <typename Sig> struct result;

		template <typename This, typename T>
		struct result< This(T) > { typedef double type; };

		explicit Vector(int sz_, double iniVal) :
			sz( sz_), data( new double[sz] ) {
			for (int i = 0; i < sz; i++) data[i] = iniVal;
		}

		// No initialization
		explicit Vector(int sz_ = 1) :
			sz( sz_), data( new double[sz] ) {
		}

		Vector(const Vector& vec) :
			sz( vec.sz), data( new double[sz] ) {
			AssignVector< AssignFunctor >()(
				vec, *this, PTT::Specified()
			);
		}

		template < typename Derived >
		Vector( const LazyVectorMaker< Derived > & maker) :
			sz( maker.columnSize()), data( new double[sz] )
		{
			maker.assignDataTo( *this);
		}

		~Vector() {
			delete [] data;
		}

		int size() const { return sz; }
		int rowSize() const { return 1; }
		int columnSize() const { return sz; }

		double dot( const Vector& vec) const
		{
			return _dot(vec, PTT::Specified());
		}

		double abs() const {
			return _abs( PTT::Specified());
		}

		// accessing to an element of this vector
		double& operator()(int i) { return data[i]; }
		const double& operator()(int i) const { return data[i]; }


		Vector& operator=( const Vector& rhs ) {
			AssignVector< AssignFunctor >()(
				rhs, *this, PTT::Specified()
			);
			return *this;
		}

		// assigning the lhs of a vector expression into this vector
		template < typename Expr >
		Vector& operator=( const ExprWrapper< Expr >& expr ) {
			// NOTE that this argument type should be ExprWrapper< >,
			// otherwise the following operator=() cannot be
			// instanciated for SparseLinAlg::LazyIterSolver
			// which is the derived class of LazyVectorMaker< > .
			AssignVecExpr< AssignFunctor >()(
					expr, VecExprTagGrammar()( expr),
					*this, PTT::Specified()
			);
			return *this;
		}

		template < typename Derived >
		Vector& operator=( const LazyVectorMaker< Derived > & maker) {
			maker.assignDataTo( *this);
			return *this;
		}

		// plus assigning the lhs of a vector expression into this vector
		template <typename Expr>
		Vector& operator+=( const ExprWrapper< Expr >& expr ) {
			// NOTE that this argument type should be ExprWrapper< >,
			// otherwise the following operator=() cannot be
			// instanciated for SparseLinAlg::LazyIterSolver
			// which is the derived class of LazyVectorMaker< > .
			AssignVecExpr< PlusAssignFunctor >()(
					expr, VecExprTagGrammar()( expr),
					*this, PTT::Specified()
			);
			return *this;
		}

		// minus assigning the lhs of a vector expression into this vector
		template <typename Expr>
		Vector& operator-=( const ExprWrapper< Expr >&expr ) {
			// NOTE that this argument type should be ExprWrapper< >,
			// otherwise the following operator=() cannot be
			// instanciated for SparseLinAlg::LazyIterSolver
			// which is the derived class of LazyVectorMaker< > .
			AssignVecExpr< MinusAssignFunctor >()(
					expr, VecExprTagGrammar()( expr),
					*this, PTT::Specified()
			);
			return *this;
		}


		friend void diagPrecondConGrad( Vector & ansVec,
				const Matrix & coeffMat, const Vector & rhsVec,
				const Vector & initGuessVec,
				double convergenceCriterion);

		template < typename AssignType > friend class AssignVecExpr;
		template < typename AssignType > friend class AssignVector;
	};


	// Function object for lazily assigning
	// an vector expresion into a vector object
	template < typename AssignType >
	struct AssignVecExpr
	{
		template < typename Expr >
		void operator()(
			const ExprWrapper< Expr >& expr, const VecExprTag&,
			Vector& lhs,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& )
		const
		{
			for(int i=0; i < lhs.sz; ++i)
				AssignType()( lhs.data[i], VecExprGrammar()( expr(i) ) );
		};

		template < typename Expr >
		void operator()(
			const ExprWrapper< Expr >& expr, const VecMapTag&,
			Vector& lhs,
			const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& )
		const
		{
			#pragma omp parallel for
			for(int i=0; i < lhs.sz; ++i)
				AssignType()( lhs.data[i], VecMapGrammar()( expr(i) ) );
			// std::cout << "skelton for Map, OpenMP " << std::endl;
		};

		template < typename Expr >
		void operator()(
			const ExprWrapper< Expr >& expr, const VecMapReduceTag&,
			Vector& lhs,
			const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& )
		const
		{
			#pragma omp parallel for shared( lhs)
			for(int i=0; i < lhs.sz; ++i)
				AssignType()( lhs.data[i], VecMapReduceGrammar()( expr(i) ) );
			// std::cout << "skelton for Map and Reduce, OpenMP " << std::endl;
		};
	};


	// Implementations for
	// function object for lazily assigning
	// an vector object (not expression temaplte) into a vector object
	template < typename AssignType >
	void AssignVector< AssignType >::operator()(
		const Vector& rhs, Vector& lhs,
		const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& )
	const
	{
		for(int i=0; i < lhs.sz; ++i)
			AssignType()( lhs.data[i], rhs.data[i] );
	};

	template < typename AssignType >
	void AssignVector< AssignType >::operator()(
		const Vector& rhs, Vector& lhs,
		const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& )
	const
	{
		#pragma omp parallel for
		for(int i=0; i < lhs.sz; ++i)
			AssignType()( lhs.data[i], rhs.data[i] );
		// std::cout << "skelton Map and Reduce, OpenMP " << std::endl;
	};


	class DiagonalMatrix;

	template < typename Derived >
	struct LazyDiagonalMatrixMaker
	{
		const int sz;

		explicit LazyDiagonalMatrixMaker( int size) : sz( size) {}
		virtual ~LazyDiagonalMatrixMaker() {}

		int rowSize() const { return sz; }
		int columnSize() const { return sz; }
		int size() const { return sz; }

		void assignDataTo(DiagonalMatrix& lhs) const {
			static_cast< const Derived & >( *this).assignDataTo_derived( lhs);
		}
	};


	class DiagonalMatrix
	{
	private:
		const int sz;
		double* data;

	public:
		template <typename Sig> struct result;

		template <typename This, typename T>
		struct result< This(T,T) > { typedef double type; };

		template <typename This, typename T>
		struct result< This(T) > { typedef double type; };

		explicit DiagonalMatrix(int sz_ = 1, double iniVal = 0.0) :
				sz( sz_), data( new double[sz] )
		{
			for (int i = 0; i < sz; i++) data[i] = iniVal;
		}

		~DiagonalMatrix() {
			delete [] data;
		}

		int size() const { return sz; }
		int rowSize() const { return sz; }
		int columnSize() const { return sz; }

		// accessing to an element of this vector
		double& operator()(int i) { return data[i]; }
		const double& operator()(int i) const { return data[i]; }

		double operator()(int ri, int ci) const {
			if ( ri == ci )  {
				return data[ri];
			} else {
				return 0.0;
			}
		}

		template< typename Expr >
		DiagonalMatrix& operator=( const ExprWrapper< Expr >& expr ) {
			for(int i=0; i < sz; ++i)
				data[i] = DiagMatExprGrammar()( expr(i) );
			return *this;
		}

		template < typename Derived >
		DiagonalMatrix&
		operator=( const LazyDiagonalMatrixMaker< Derived >& maker ) {
			maker.assignDataTo( *this);
			return *this;
		}
	};


	template < typename Derived >
	struct LazyMatrixMaker
	{
		const int rowSz, colSz;

		explicit LazyMatrixMaker( int rowSize, int columnSize) :
				rowSz( rowSize), colSz( columnSize) {}
		virtual ~LazyMatrixMaker() {}

		int rowSize() const { return rowSz; }
		int columnSize() const { return colSz; }

		void assignDataTo(Matrix& lhs) const {
			static_cast< const Derived & >( *this).assignDataTo_derived( lhs);
		}
	};


	class Matrix
	{
	private:
		int rowSz, colSz;
		double* data;
		double** m;

	public:
		template <typename Sig> struct result;

		template <typename This, typename T>
		struct result< This(T,T) > { typedef double type; };

		explicit Matrix(int rowSize = 1, int columnSize =1,
					double iniVal = 0.0) :
			rowSz( rowSize), colSz(columnSize),
			data( new double[rowSz*colSz] ), m( new double*[rowSz])
		{
			for (int i = 0; i < rowSz; i++) m[i] = data + i*colSz;
			for (int ri = 0; ri < rowSz; ri++)
				for (int ci = 0; ci < colSz; ci++) m[ri][ci] = iniVal;
		}

		Matrix( const Matrix& mat) :
			rowSz( mat.rowSz), colSz( mat.colSz),
			data( new double[rowSz*colSz] ), m( new double*[rowSz])
		{
			for (int i = 0; i < rowSz; i++) m[i] = data + i*colSz;
				for (int ri = 0; ri < rowSz; ri++)
					for (int ci = 0; ci < colSz; ci++)
						m[ri][ci] = mat.m[ri][ci];
		}

		~Matrix()
		{
			delete [] m;
			delete [] data;
		}

		int rowSize() const { return rowSz; }
		int columnSize() const { return colSz; }

		// accesing to a matrix element
		double& operator()(int ri, int ci) { return m[ri][ci]; }
		const double& operator()(int ri, int ci) const { return m[ri][ci]; }

		// assigning the lhs of a vector expression into this matrix
		template<typename Expr>
		Matrix& operator=( const ExprWrapper< Expr >& expr ) {
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] = MatExprGrammar()( expr(ri, ci) );
			return *this;
		}

		template < typename Derived >
		Matrix& operator=( const LazyMatrixMaker< Derived >& maker) {
			maker.assignDataTo( *this);
			return *this;
		}

		// assigning and adding the lhs of a vector expression into this matrix
		template<typename Expr>
		Matrix& operator+=( const Expr& expr ) {
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] += MatExprGrammar()( expr(ri, ci) );
			return *this;
		}

		// assigning and subtracting the lhs of a vector expression into
		// this matrix
		template<typename Expr>
		Matrix& operator-=( const Expr& expr ) {
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] -= MatExprGrammar()( expr(ri, ci) );
			return *this;
		}

		friend void diagPrecondConGrad( Vector & ansVec,
				const Matrix & coeffMat, const Vector & rhsVec,
				const Vector & initGuessVec,
				double convergenceCriterion);
	};


}




#endif /* DENSELINALG_MATRIXVECTOR_HPP_ */
