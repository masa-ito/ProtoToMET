/*
 * matVecMultAndVecSubMetaOpenMP.hpp
 *
 *  Created on: 2016/05/16
 *      Author: Masakatsu ITO
 */

#ifndef MATVECMULTANDVECSUBMETAOPENMP_HPP_
#define MATVECMULTANDVECSUBMETAOPENMP_HPP_

#include <iostream>
#include <boost/proto/proto.hpp>

#include <ParallelizationTypeTag/Default.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

namespace PTT = ParallelizationTypeTag;

namespace DenseLinAlg {

	class Vector;
	class Matrix;


	// Callable transform object to make a proto exression
	// for lazily evaluating multiplication
	struct MatVecMult;

	// The grammar for the multiplication of a matrix and a vector
	struct MatVecMultGrammar : proto::or_<
		proto::when<
			proto::multiplies< proto::terminal< Matrix> ,
								proto::terminal< Vector> >,
			MatVecMult( proto::_value( proto::_left),
						proto::_value( proto::_right) )
		>
	> {};

	// The transformation rule for vector element expressions
	// This transform accepts a subscript index  of an expression being parsed
	// as the state variable,and distribute that index over the child nodes.
	struct VecElmGrammar : proto::or_<
		// Vector
		proto::when< proto::terminal< Vector>,
					proto::_make_function( proto::_, proto::_state) >,
		// Matrix * Vector
		proto::when< MatVecMultGrammar,
					proto::_make_function( MatVecMultGrammar( proto::_),
											proto::_state) >,
		// double * Vector

		// DiagonalMatrix * Vector

		// VecElmGrammar +(-) VecElmGrammar
		proto::plus< VecElmGrammar, VecElmGrammar> ,
		proto::minus< VecElmGrammar, VecElmGrammar>
	> {};

	// The grammar for a vector expression
	struct VecExprGrammar : proto::or_<
		// VecElmGrammar( index )
		proto::when<
			proto::function< VecElmGrammar, proto::_ >,
			proto::_default< >( VecElmGrammar(proto::_left, proto::_right) )
		>,

		// Vector
		proto::terminal< Vector >,

		// VecExprGrammar +(-) VecExprGrammar
		proto::plus< VecExprGrammar, VecExprGrammar> ,
		proto::minus< VecExprGrammar, VecExprGrammar>,

		// Vector * double , or double * Vector

		// Matrix * Vector
		MatVecMultGrammar //,

		// DiagonalMatrix * VecExprGrammar
	> {};


	// The transformation rule for matrix element expressions
	struct MatElmGrammar : proto::or_<
		// Matrix
		proto::when<
			proto::terminal< Matrix >,
			proto::_make_function( proto::_,
										proto::_state, proto::_data)
		> // ,

		// Matrix * DiagonalMatrix * Matrix

		// MatElmGrammar +(-) MatElmGrammar
	> {};


	// The tranformation rule for matrix expressions
	struct MatExprGrammar : proto::or_<
		// MatElmGrammar( rowIndex, columnIndex )
		proto::when<
				proto::function< MatElmGrammar, proto::_ , proto::_ >,
				proto::_default< >(
						MatElmGrammar(proto::_child0,
								proto::_child1, proto::_child2)
				)
			>,
		// Matrix
		proto::terminal< Matrix > //,

		// MatExprGrammar * MatExprGrammar
		// MatExprGrammar * DiagMatExprGrammar
		// DigaMatExprGrammar * MatExprGrammar
		// MatExprGrammar +(-) MatExprGrammar
	> {};

	// The tranformation rule for linear algebraic expressions
	struct ExprGrammar : proto::or_<
		VecExprGrammar,
		MatExprGrammar // ,
		// DiagMatExprGrammar
	> {};


	template <typename GrammarType>
	struct GrammarChecker
	{
		template <class Expr>
		void operator ()(Expr const & expr) const {
			static_assert(
					proto::matches<Expr, GrammarType>::value,
					"The expression does not match to the grammar!"
			);
			proto::display_expr( expr );
			std::cout << std::endl;
		}
	};


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
		explicit ExprWrapper(const ExprType& e)
			: proto::extends<ExprType, ExprWrapper<ExprType>, Domain>(e) /*,
			  rowSz( e.rowSize()), colSz( e.columnSize()) */
		{}

	};


	class Vector {
	private:
		const int sz;
		double* data;

	public:
		template <typename Sig> struct result;

		template <typename This, typename T>
		struct result< This(T) > { typedef double type; };

		explicit Vector(int sz_, double iniVal) :
			sz( sz_), data( new double[sz] ) {
			for (int i = 0; i < sz; i++) data[i] = iniVal;
			// std::cout << "Created" << std::endl;
		}

		// No initialization
		explicit Vector(int sz_ = 1) :
			sz( sz_), data( new double[sz] ) {
			// std::cout << "Created" << std::endl;
		}

		Vector(const Vector& vec) :
			sz( vec.sz), data( new double[sz] ) {
			for (int i = 0; i < sz; i++) data[i] = vec.data[i];
			// std::cout << "Copied! " << std::endl;
		}

		/* template < typename Derived >
		Vector( const LazyVectorMaker< Derived > & maker) :
			sz( maker.columnSize()), data( new double[sz] )
		{
			maker.assignDataTo( *this);
		} */

		~Vector() {
			delete [] data;
			// std::cout << "Deleted" << std::endl;
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
			// proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] = VecExprGrammar()( expr(i) );
				// data[i] = trans( VecExprGrammar()( expr(i) ) );
			return *this;
		}

		/* template < typename Derived >
		Vector& operator=( const LazyVectorMaker< Derived > & maker) {
			maker.assignDataTo( *this);
			return *this;
		} */


		// assigning and adding the lhs of a vector expression into this vector
		template<typename Expr>
		Vector& operator+=( const Expr& expr ) {
			// proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] += VecExprGrammar()( expr(i) );
				// data[i] += trans( VecExprGrammar()( expr(i) ) );
			return *this;
		}

		// assigning and subtracting the lhs of a vector expression into
		// this vector
		template<typename Expr>
		Vector& operator-=( const Expr& expr ) {
			// proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] -= VecExprGrammar()( expr(i) );
				// data[i] -= trans( VecExprGrammar()( expr(i) ) );
			return *this;
		}

		/* friend void diagPrecondConGrad( Vector & ansVec,
				const Matrix & coeffMat, const Vector & rhsVec,
				const Vector & initGuessVec,
				double convergenceCriterion); */

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

		/* template < typename Derived >
		Matrix& operator=( const LazyMatrixMaker< Derived >& maker) {
			maker.assignDataTo( *this);
			return *this;
		} */

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

		/* friend void diagPrecondConGrad( Vector & ansVec,
				const Matrix & coeffMat, const Vector & rhsVec,
				const Vector & initGuessVec,
				double convergenceCriterion); */
	};


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


	// Define a trait for detecting linear algebraic terminals, to be used
	// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
	template<typename> struct IsExpr  : mpl::false_ {};

	template<> struct IsExpr< Vector > : mpl::true_  {};
	template<> struct IsExpr< Matrix > : mpl::true_  {};
	// template<> struct IsExpr< DiagonalMatrix > : mpl::true_  {};

	template<> struct IsExpr< LazyMatVecMult > : mpl::true_  {};

	// template<> struct IsExpr< LazyMatDiagmatMatMult > : mpl::true_  {};

	// This defines all the overloads to make expressions involving
	// Vector and Matrix objects to build Proto's expression templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsExpr, Domain)

}


namespace DLA = DenseLinAlg;

void testMatVecMultAndVecAdd()
{
	using namespace DenseLinAlg;

    Matrix matA( 3, 3);
    Vector vecX(3), vecB(3), vecR(3);

    matA(0,0) = 1.00; matA(0,1) = 1.01; matA(0,2) = 1.02;
    matA(1,0) = 1.10; matA(1,1) = 1.11; matA(1,2) = 1.12;
    matA(2,0) = 1.20; matA(2,1) = 1.21; matA(2,2) = 1.22;

    vecX(0) = 1.0;
    vecX(1) = 2.0;
    vecX(2) = 3.0;

    vecB(0) = 4.0;
    vecB(1) = 5.0;
    vecB(2) = 6.0;

	GrammarChecker< ExprGrammar >
		checker = GrammarChecker< ExprGrammar >();

    std::cout << "Checking if (vecB - matA * vecX) matches to ExprGrammar ..."
        	<< std::endl;
	checker( vecB - matA * vecX );

    std::cout << "Checking if ( vecB - matA * vecX)(2) "
    		<< "matches to ExprGrammar ..."
        	<< std::endl;
	checker( ( vecB - matA * vecX)(2) );

    double elm2 = VecExprGrammar()( ( vecB - matA * vecX)(2) );

    std::cout << "( vecB - matA * vecX)(2) = " << elm2 << std::endl;
    // This should be  -1.28 .

    vecR = vecB - matA * vecX;

    std::cout << " vecR = vecB - matA * vecX = " << std::endl;
    std::cout << " ( " << vecR(0) << ", " << vecR(1) << ", " <<
    		vecR(2) << ")" << std::endl;
    // This should be ( -2.08 , -1.68 , -1.28) .

	return;
}





#endif /* MATVECMULTANDVECSUBMETAOPENMP_HPP_ */
