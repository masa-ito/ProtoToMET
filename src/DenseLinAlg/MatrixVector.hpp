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

#include <DenseLinAlg/Grammar.hpp>
// #include <SparseLinAlg/IterSolver.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


namespace DenseLinAlg {


	// A wrapper for a linear algebraic expression
	template<typename E> struct ExprWrapper;

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
		/* typedef double result_type; */
		const int rowSz, colSz;

		explicit ExprWrapper(const ExprType& e)
			: proto::extends<ExprType, ExprWrapper<ExprType>, Domain>(e),
			  rowSz( e.rowSize()), colSz( e.columnSize())
		{}

		int rowSize() const { return rowSz; }
		int columnSize() const { return colSz; }
	};



	class Vector {
		private:
			const int sz;
			double* data;

	public:
		template <typename Sig> struct result;

		template <typename This, typename T>
		struct result< This(T) > { typedef double type; };

		explicit Vector(int sz_ = 1, double iniVal = 0.0) :
			sz( sz_), data( new double[sz] ) {
			for (int i = 0; i < sz; i++) data[i] = iniVal;
			std::cout << "Created" << std::endl;
		}

		Vector(const Vector& vec) :
			sz( vec.sz), data( new double[sz] ) {
			for (int i = 0; i < sz; i++) data[i] = vec.data[i];
			std::cout << "Copied! " << std::endl;
		}

		// assigning the lhs of a vector expression into this vector
		template<typename Expr>
		Vector( const Expr& expr ) :
			sz( expr.columnSize()), data( new double[sz] ) {
			proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] = trans( VecExprGrammar()( expr(i) ) );
		}

		Vector( const LazyVectorMaker & maker) :
			sz( maker.columnSize()), data( new double[sz] )
		{
			maker.assignDataTo( *this);
		}

//		template <typename LazySolverType>
//		Vector(LazySolverType & solver) :
//			sz( solver.rhs.size()), data( new double[sz] )
//		{
//			solver.solveAndAssignTo( *this);
//		}

		~Vector() {
			delete [] data;
			std::cout << "Deleted" << std::endl;
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
		template<typename Expr>
		Vector& operator=( const Expr& expr ) {
			proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] = trans( VecExprGrammar()( expr(i) ) );
			return *this;
		}

		Vector& operator=( const LazyVectorMaker & maker) {
			maker.assignDataTo( *this);
			return *this;
		}

//		template <typename LazySolverType>
//		Vector& operator=(LazySolverType & solver) {
//			solver.solveAndAssignTo( *this);
//			return *this;
//		}

		// assigning and adding the lhs of a vector expression into this vector
		template<typename Expr>
		Vector& operator+=( const Expr& expr ) {
			proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] += trans( VecExprGrammar()( expr(i) ) );
			return *this;
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

		DiagonalMatrix( const LazyDiagonalMatrixMaker & maker) :
			sz( maker.columnSize()), data( new double[sz] )
		{
			maker.assignDataTo( *this);
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

		template<typename Expr>
		DiagonalMatrix& operator=( const Expr& expr ) {
			proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] = trans( DiagMatExprGrammar()( expr(i) ) );
			return *this;
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
			std::cout << "Created" << std::endl;
		}

		Matrix( const Matrix& mat) :
			rowSz( mat.rowSz), colSz( mat.colSz),
			data( new double[rowSz*colSz] ), m( new double*[rowSz])
		{
			for (int i = 0; i < rowSz; i++) m[i] = data + i*colSz;
				for (int ri = 0; ri < rowSz; ri++)
					for (int ci = 0; ci < colSz; ci++)
						m[ri][ci] = mat.m[ri][ci];
				std::cout << "Copied! " << std::endl;
		}

		Matrix( const LazyMatrixMaker & maker) :
			rowSz( maker.rowSize()), colSz( maker.columnSize()),
			data( new double[rowSz*colSz] ), m( new double*[rowSz])
		{
			for (int i = 0; i < rowSz; i++) m[i] = data + i*colSz;
			maker.assignDataTo( *this);
		}

		~Matrix()
		{
			delete [] m;
			delete [] data;
			std::cout << "Deleted" << std::endl;
		}

		int rowSize() const { return rowSz; }
		int columnSize() const { return colSz; }

		// accesing to a vector element
		double& operator()(int ri, int ci) { return m[ri][ci]; }
		const double& operator()(int ri, int ci) const { return m[ri][ci]; }

		// assigning the lhs of a vector expression into this vector
		template<typename Expr>
		Matrix& operator=( const Expr& expr ) {
			proto::_default<> trans;
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] = trans( MatExprGrammar()( expr(ri, ci) ) );
			return *this;
		}

		// assigning and adding the lhs of a vector expression into this vector
		template<typename Expr>
		Matrix& operator+=( const Expr& expr ) {
			proto::_default<> trans;
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] += trans( MatExprGrammar()( expr(ri, ci) ) );
			return *this;
		}
	};


}




#endif /* DENSELINALG_MATRIXVECTOR_HPP_ */
