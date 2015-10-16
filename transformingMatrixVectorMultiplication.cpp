/*
 * transformingMatrixVectorMultiplication.cpp
 *
 *  Created on: 2015/09/30
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

namespace LinAlg {
	class Vector;
	class Matrix;


	// Callable transform object to make a proto exression
	// for lazily evaluationg a. multiplication
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
		// VecElmGrammar +(-) VecElmGrammar
		proto::plus< VecElmGrammar, VecElmGrammar> ,
		proto::minus< VecElmGrammar, VecElmGrammar>
	> {};

	// The grammar for a vector expression
	struct VecExprGrammar : proto::or_<
		// VecElmGrammar( index )
		proto::when< proto::function< VecElmGrammar, proto::_ >,
					VecElmGrammar(proto::_left, proto::_right) >,
		// Vector
		proto::terminal< Vector >,
		// VecExprGrammar +(-) VecExprGrammar
		proto::plus< VecExprGrammar, VecExprGrammar> ,
		proto::minus< VecExprGrammar, VecExprGrammar>,
		// Matrix * Vector
		proto::function< MatVecMultGrammar, proto::_>,
		MatVecMultGrammar
	> {};

	// The transformation rule for matrix element expressions
	struct MatElmGrammar : proto::or_<
		// Matrix
		proto::when< proto::terminal< Matrix>,
					proto::_make_function( proto::_,
										proto::_state, proto::_data) >,
		// MatElmGrammar +(-) MatElmGrammar
		proto::plus< MatElmGrammar, MatElmGrammar> ,
		proto::minus< MatElmGrammar, MatElmGrammar>
	> {};

	// The tranformation rule for matrix expressions
	struct MatExprGrammar : proto::or_<
		// MatElmGrammar( rowIndex, columnIndex )
		proto::when<
				proto::function< MatElmGrammar, proto::_ , proto::_ >,
				MatElmGrammar(proto::_child0, proto::_child1, proto::_child2)
			>,
		// matrix
		proto::terminal< Matrix >,
		// MatExprGrammar +(-) MatExprGrammar
		proto::plus< MatExprGrammar, MatExprGrammar> ,
		proto::minus< MatExprGrammar, MatExprGrammar>
	> {};

	// The tranformation rule for linear algebraic expressions
	struct ExprGrammar : proto::or_<
		VecExprGrammar,
		MatExprGrammar
	> {};

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

		explicit ExprWrapper(const ExprType& e)
			: proto::extends<ExprType, ExprWrapper<ExprType>, Domain>(e)
		{}
	};


	// Testing if data in an heap array can be a vector object
	class Vector {
		private:
			int sz;
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

		~Vector() {
			delete [] data;
			std::cout << "Deleted" << std::endl;
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

		// assigning and adding the lhs of a vector expression into this vector
		template<typename Expr>
		Vector& operator+=( const Expr& expr ) {
			proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] += trans( VecExprGrammar()( expr(i) ) );
			return *this;
		}
	};


	// Testing if data in an heap array can be a matrix object
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

		Matrix(const Matrix& mat) :
			rowSz( mat.rowSz), colSz( mat.colSz),
			data( new double[rowSz*colSz] ), m( new double*[rowSz])
		{
			for (int i = 0; i < rowSz; i++) m[i] = data + i*colSz;
				for (int ri = 0; ri < rowSz; ri++)
					for (int ci = 0; ci < colSz; ci++)
						m[ri][ci] = mat.m[ri][ci];
				std::cout << "Copied! " << std::endl;
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

	// Lazy function object for evaluating an element of
	// the resultant vector from the multiplication of
	// a matrix and vector objects.
	//
	// An expression like ( matrix * vector )(index) is transformed
	// into the loop for calculating the dot product between
	// the index'th row of the matrix and the vector.
	struct LazyMatVecMult
	{
		Matrix const& m;
		Vector const& v;
		int mColSz;

		typedef double result_type;
		// typedef mpl::int_<1> proto_arity;

		explicit LazyMatVecMult(Matrix const& mat, Vector const& vec) :
		m( mat), v( vec), mColSz(mat.rowSize()) {}

		LazyMatVecMult(LazyMatVecMult const& lazy) :
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
	template<> struct IsExpr< Vector> : mpl::true_  {};
	template<> struct IsExpr< Matrix> : mpl::true_  {};
	// template<> struct IsExpr< MatVecMult> : mpl::true_  {};
	template<> struct IsExpr< LazyMatVecMult> : mpl::true_  {};

	// This defines all the overloads to make expressions involving
	// Vector and Matrix objects to build Proto's expression templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsExpr, Domain)
	// BOOST_PROTO_DEFINE_OPERATORS(IsExpr, proto::default_domain)

}


template <typename SyntaxRule>
struct ExpressionSyntaxChecker
{
	template <class Expr>
	void operator ()(Expr const & expr) const {
		static_assert(
				proto::matches<Expr, SyntaxRule>::value,
				"The expression does not match to the syntax rule!"
		);
		proto::display_expr( expr );
		std::cout << std::endl;
	}
};



void testVecAdd()
{
	using namespace LinAlg;

    // lazy_vectors with 3 elements each.
    Vector v1( 3, 1.0 ), v2( 3, 2.0 ), v3( 3, 3.0 );

	ExpressionSyntaxChecker< ExprGrammar >
		syntaxChecker = ExpressionSyntaxChecker< ExprGrammar >();

    // Add two vectors lazily and get the 2nd element.
    std::cout << "Checking if v2 + v3 matches to ExprGrammar ..."
    		<< std::endl;
    syntaxChecker( v2 + v3 );

    std::cout << "Checking if (v2 + v3)(2) matches to ExprGrammar ..."
    		<< std::endl;
    syntaxChecker( ( v2 + v3 )( 2 ) );

    std::cout << "Checking if VecExprOpt()( ( v2 + v3 )( 2 ) )";
    std::cout << " matches to VecExprOpt rule ..." << std::endl;
    syntaxChecker( ExprGrammar()( ( v2 + v3 )( 2 ) ) );

    proto::_default<> trans;

    // Look ma, no temporaries!
    double d1 = trans( ExprGrammar()( ( v2 + v3 )( 2 ) ) );
    std::cout << "( v2 + v3 )( 2 ) = " << d1 << std::endl;

    // Subtract two vectors and add the result to a third vector.
    v1 += v2 - v3;                  // Still no temporaries!
    std::cout << "v1 += v2 - v3" << std::endl;
    std::cout << "v1 =";
    std::cout << '{' << v1(0) << ',' << v1(1)
              << ',' << v1(2) << ',' << v1(3) << '}' << std::endl;

    return;
}

void testMatAdd()
{
	using namespace LinAlg;

	Matrix m1( 3, 3), m2( 3, 3), m3( 3, 3);
	for (int ri=0; ri < 3; ri++)
		for (int ci=0; ci < 3; ci++)
		{
			m1(ri, ci) = 1.0 + 0.1 * ri + 0.01 * ci;
			m2(ri, ci) = 2.0 + 0.1 * ri + 0.01 * ci;
		}

	ExpressionSyntaxChecker< ExprGrammar >
		syntaxChecker = ExpressionSyntaxChecker< ExprGrammar >();

	std::cout << "Checking if m1 + m2 matches to ExprGrammar ..."
			<< std::endl;
	syntaxChecker( m1 + m2 );

	std::cout << "Checking if (m1 + m2)(0,1) matches to ExprGrammar ..."
			<< std::endl;
	syntaxChecker( (m1 + m2)(0,1) );

    proto::_default<> trans;
	double elm01 = trans( ExprGrammar()( (m1 + m2)(0,1) ) );
	std::cout << " (m1 + m2)(0,1) = " << elm01 << std::endl;

	m3 = m1 + m2;

	//  !!! these should be modified !!!!!
	std::cout << "m3 = " << std::endl;
	std::cout << "( ( " << m3(0,0) << "    " << m3(0,1) << " "
			<< m3(0,2) << " ) " << std::endl;
	std::cout << "  ( " << m3(1,0) << "  " << m3(1,1) << " " <<
			m3(1,2) << " ) " << std::endl;
	std::cout << "  ( " << m3(2,0) << "  " << m3(2,1) << " " <<
			m3(2,2) << " ) )" << std::endl;

	return;
}

void testMatVecMul()
{
	using namespace LinAlg;

    Matrix mat( 3, 3);
    Vector vec1(3), vec2(3);

    mat(0,0) = 1.00; mat(0,1) = 1.01; mat(0,2) = 1.02;
    mat(1,0) = 1.10; mat(1,1) = 1.11; mat(1,2) = 1.12;
    mat(2,0) = 1.20; mat(2,1) = 1.21; mat(2,2) = 1.22;

    vec1(0) = 1.0;
    vec1(1) = 2.0;
    vec1(2) = 3.0;

	ExpressionSyntaxChecker< ExprGrammar >
		syntaxChecker = ExpressionSyntaxChecker< ExprGrammar >();

    std::cout << "Checking if mat * vec1 matches to ExprGrammar ..."
        	<< std::endl;
	syntaxChecker( mat * vec1 );

    /* std::cout << "Checking if MatVecMultGrammar()( mat * vec1) "
    		<< "matches to ExprGrammar ..."
        	<< std::endl;
	syntaxChecker( MatVecMultGrammar()( mat * vec1) ); */

    std::cout << "Checking if ( mat * vec1)(2) matches to ExprGrammar ..."
        	<< std::endl;
	syntaxChecker( ( mat * vec1)(2) );

    /* std::cout << "Checking if VecExprGrammar()( ( mat * vec1)(2) ) "
    		<< "matches to ExprGrammar ..."
        	<< std::endl;
	syntaxChecker( VecExprGrammar()( ( mat * vec1)(2) ) ); */

    proto::_default<> trans;
    double elm2 = trans( VecExprGrammar()( ( mat * vec1)(2) ) );

    std::cout << "( mat * vec1)(2) = " << elm2 << std::endl;
    // This should be 7.28 .

    vec2 = mat * vec1;

    std::cout << " vec2 = mat * vec1 = " << std::endl;
    std::cout << " ( " << vec2(0) << ", " << vec2(1) << ", " <<
    		vec2(2) << ")" << std::endl;
    // This should be ( 6.08 , 6.68 , 7.28) .

	return;
}

int main()
{
    std::cout << "Let's see if any temporary oject is copied !" << std::endl;

	testVecAdd();
	testMatAdd();
	testMatVecMul();

    return 0;
}

