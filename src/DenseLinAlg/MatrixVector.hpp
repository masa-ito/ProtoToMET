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

	// struct SparseLinAlg::LazyIterSolver;
	// struct SparseLinAlg::LazyPreconditioner;

	class Matrix;

	void diagPrecondConGrad( Vector & ansVec,
			const Matrix & coeffMat, const Vector & rhsVec,
			const Vector & initGuessVec,
			double convergenceCriterion);


	class Vector {
	private:
		const int sz;
		double* data;

		// Assignment in single thread
		template < typename Expr >
		void assign( const Expr& expr ,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& ) {
			for(int i=0; i < sz; ++i)
				data[i] = VecExprGrammar()( expr(i) );
		}

		// Assignment of elementwise expression in OpenMP
		template < typename Expr >
		typename boost::enable_if<
			proto::matches< Expr, VecElementwiseGrammar >
		>::type
		assign( const Expr& expr ,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) {
			#pragma omp parallel for
			for(int i=0; i < sz; ++i)
				data[i] = VecElementwiseGrammar()( expr(i) );

			std::cout << "OpenMP elementwise assign" << std::endl;
		}

		// Assignment of reduction expression in OpenMP
		template < typename Expr >
		typename boost::enable_if<
			proto::matches< Expr,VecReductionGrammar >
		>::type
		assign( const Expr& expr ,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) {
			for(int i=0; i < sz; ++i)
				data[i] = VecReductionOmpGrammar()( expr(i) );

			std::cout << "OpenMP reduction assign" << std::endl;
		}

		// Plus & Assignment in single thread
		template < typename Expr >
		void plusAssign( const Expr& expr ,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& ) {
			for(int i=0; i < sz; ++i)
				data[i] += VecExprGrammar()( expr(i) );
		}

		// Plus & Assignment of elementwise expression in OpenMP
		template < typename Expr >
		typename boost::enable_if<
			proto::matches< Expr, VecElementwiseGrammar >
		>::type
		plusAssign( const Expr& expr ,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) {
			#pragma omp parallel for
			for(int i=0; i < sz; ++i)
				data[i] += VecElementwiseGrammar()( expr(i) );

			std::cout << "OpenMP elementwise plus assign" << std::endl;
		}

		// Plus & Assignment of reduction expression in OpenMP
		template < typename Expr >
		typename boost::enable_if<
			proto::matches< Expr,VecReductionGrammar >
		>::type
		plusAssign( const Expr& expr ,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) {
			for(int i=0; i < sz; ++i)
				data[i] += VecReductionOmpGrammar()( expr(i) );

			std::cout << "OpenMP reduction plus assign" << std::endl;
		}

		// Minus & Assignment in single thread
		template < typename Expr >
		void minusAssign( const Expr& expr ,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& ) {
			for(int i=0; i < sz; ++i)
				data[i] -= VecExprGrammar()( expr(i) );
		}

		// Minus & Assignment of elementwise expression in OpenMP
		template < typename Expr >
		typename boost::enable_if<
			proto::matches< Expr, VecElementwiseGrammar >
		>::type
		minusAssign( const Expr& expr ,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) {
			#pragma omp parallel for
			for(int i=0; i < sz; ++i)
				data[i] -= VecElementwiseGrammar()( expr(i) );

			std::cout << "OpenMP elementwise minus assign" << std::endl;
		}

		// Plus & Assignment of reduction expression in OpenMP
		template < typename Expr >
		typename boost::enable_if<
			proto::matches< Expr,VecReductionGrammar >
		>::type
		minusAssign( const Expr& expr ,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) {
			for(int i=0; i < sz; ++i)
				data[i] -= VecReductionOmpGrammar()( expr(i) );

			std::cout << "OpenMP reduction minus assign" << std::endl;
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
			for (int i = 0; i < sz; i++) data[i] = vec.data[i];
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
			double d = data[0] * vec.data[0];
			for (int i = 1; i < sz; i++) d += data[i] * vec.data[i];
			return d;
		}

		double abs() const {
			double aSqr = data[0] * data[0];
			for (int i = 1; i < sz; i++) aSqr += data[i] * data[i];
			return sqrt( aSqr);
		}


		// accessing to an element of this vector
		double& operator()(int i) { return data[i]; }
		const double& operator()(int i) const { return data[i]; }


		// assigning the lhs of a vector expression into this vector
		template < typename Expr >
		Vector& operator=( const ExprWrapper< Expr >& expr ) {
			// NOTE that this argument type should be ExprWrapper< >,
			// otherwise the following operator=() cannot be
			// instanciated for SparseLinAlg::LazyIterSolver
			// which is the derived class of LazyVectorMaker< > .

			assign( expr, PTT::Specified() );
			return *this;
		}

		template < typename Derived >
		Vector& operator=( const LazyVectorMaker< Derived > & maker) {
			maker.assignDataTo( *this);
			return *this;
		}

		// Adding and assigning the lhs of a vector expression into
		// this vector
		template < typename Expr >
		Vector& operator+=( const Expr& expr ) {
			plusAssign( expr, PTT::Specified() );
			return *this;
		}

		// Subtracting and assigning the lhs of a vector expression into
		// this vector
		template < typename Expr >
		Vector& operator-=( const Expr& expr ) {
			minusAssign( expr, PTT::Specified() );
			return *this;
		}


		friend void diagPrecondConGrad( Vector & ansVec,
				const Matrix & coeffMat, const Vector & rhsVec,
				const Vector & initGuessVec,
				double convergenceCriterion);

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
