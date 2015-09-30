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

class Vector;
class Matrix;


// The transformation rule for vector element expressions
// This transform accepts a subscript index  of an expression being parsed
// as the state variable,and distribute that index over the child nodes.
struct VecElmTrans : proto::or_<
	proto::when< proto::terminal< Vector>,
				proto::_make_function( proto::_, proto::_state) >,
	proto::plus< VecElmTrans, VecElmTrans> ,
	proto::minus< VecElmTrans, VecElmTrans>
> {};

// The transformation rule for matrix element expressions
struct MatElmTrans : proto::or_<
	proto::when< proto::terminal< Matrix>,
				proto::_make_function( proto::_,
									proto::_state, proto::_data) >,
	proto::plus< MatElmTrans, MatElmTrans> ,
	proto::minus< MatElmTrans, MatElmTrans>
> {};

// The tranformation rule for vector expressions
struct VecExprTrans : proto::or_<
	proto::when< proto::function< VecElmTrans, proto::_ >,
				VecElmTrans(proto::_left, proto::_right) >,
	proto::terminal< Vector >,
	proto::plus< VecExprTrans, VecExprTrans> ,
	proto::minus< VecExprTrans, VecExprTrans>
> {};


// The tranformation rule for matrix expressions
struct MatExprTrans : proto::or_<
	proto::when<
			proto::function< MatElmTrans, proto::_ , proto::_ >,
			MatElmTrans(proto::_child0, proto::_child1, proto::_child2)
		>,
		proto::terminal< Matrix >,
		proto::plus< MatExprTrans, MatExprTrans> ,
		proto::minus< MatExprTrans, MatExprTrans>
> {};

// The tranformation rule for linear algebraic expressions
struct LinAlgExprTrans : proto::or_<
	VecExprTrans,
	MatExprTrans
> {};


template<typename Expr> struct LinAlgExpr;

// The above grammar is associated with this domain.
struct LinAlgDomain
	: proto::domain<proto::generator<LinAlgExpr>, LinAlgExprTrans>
{};

//
// Linear Algebraic Expression Templates
//
template<typename Expr>
struct LinAlgExpr
	: proto::extends<Expr, LinAlgExpr<Expr>, LinAlgDomain>
{
	explicit LinAlgExpr(const Expr& e)
		: proto::extends<Expr, LinAlgExpr<Expr>, LinAlgDomain>(e)
	{}
};


//
// Vector data are stored in an heap array.
//
class Vector {
	private:
		int sz;
		double* data;

public:
	template <typename Signature>
	struct result;

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
		std::cout << "Copied" << std::endl;
	}

	~Vector() {
		delete [] data;
		std::cout << "Deleted" << std::endl;
	}

	// accesing to a vector element
	double& operator()(int i) { return data[i]; }
	const double& operator()(int i) const { return data[i]; }

	// assigning the lhs of a vector expression into this vector
	template<typename Expr>
	Vector& operator=( const Expr& expr ) {
		proto::_default<> trans;
		for(int i=0; i < sz; ++i)
			data[i] = trans( VecExprTrans()( expr(i) ) );
		return *this;
	}

	// assigning and adding the lhs of a vector expression into this vector
	template<typename Expr>
	Vector& operator+=( const Expr& expr ) {
		proto::_default<> trans;
		for(int i=0; i < sz; ++i)
			data[i] += trans( VecExprTrans()( expr(i) ) );
		// std::cout << "Vector& operator+= done." << std::endl;
		return *this;
	}
};

//
// Matrix data are stored in an heap array.
//
class Matrix
{
private:
	int rowSz, colSz;
	double* data;
	double** m;

public:
	template <typename Signature> struct result;

	template <typename This, typename T>
	struct result< This(T,T) > { typedef double type; };

	explicit Matrix(int rowSize = 1, int columnSize =1, double iniVal = 0.0) :
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
		std::cout << "Copied" << std::endl;
	}

	~Matrix()
	{
		delete [] m;
		delete [] data;
		std::cout << "Deleted" << std::endl;
	}

	// accesing to a vector element
	double& operator()(int ri, int ci) { return m[ri][ci]; }
	const double& operator()(int ri, int ci) const { return m[ri][ci]; }

	// assigning the lhs of a vector expression into this vector
	template<typename Expr>
	Matrix& operator=( const Expr& expr ) {
		proto::_default<> trans;
		for(int ri=0; ri < rowSz; ri++)
			for (int ci=0; ci < colSz; ci++ )
				//m[ri][ci] = trans( expr(ri, ci) );
				m[ri][ci] = trans( MatExprTrans()( expr(ri, ci) ) );
		return *this;
	}

	// assigning and adding the lhs of a vector expression into this vector
	template<typename Expr>
	Matrix& operator+=( const Expr& expr ) {
		proto::_default<> trans;
		for(int ri=0; ri < rowSz; ri++)
			for (int ci=0; ci < colSz; ci++ )
				// m[ri][ci] += trans( expr(ri, ci) );
				m[ri][ci] += trans( MatExprTrans()( expr(ri, ci) ) );
		return *this;
	}
};


// Define a trait for detecting linear algebraic terminals, to be used
// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
template<typename> struct IsLinAlg  : mpl::false_ {};
template<> struct IsLinAlg< Vector> : mpl::true_  {};
template<> struct IsLinAlg< Matrix> : mpl::true_  {};


namespace LinAlgOps {
	// This defines all the overloads to make expressions involving
	// Vector and Matrix objects to build linear algebraic expression
	// templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsLinAlg, LinAlgDomain)
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


int main()
{
	using namespace LinAlgOps;

	ExpressionSyntaxChecker< LinAlgExprTrans >
		syntaxChecker = ExpressionSyntaxChecker< LinAlgExprTrans >();
    proto::_default<> trans;

    // lazy_vectors with 4 elements each.
    Vector v1( 4, 1.0 ), v2( 4, 2.0 ), v3( 4, 3.0 );

    // Add two vectors lazily and get the 2nd element.
    std::cout << "Checking if v2 + v3 matches to VecExprOpt rule ..."
    		<< std::endl;
    syntaxChecker( v2 + v3 );

    std::cout << "Checking if (v2 + v3)[2] matches to VecExprOpt rule ..."
    		<< std::endl;
    syntaxChecker( ( v2 + v3 )( 2 ) );

    std::cout << "Checking if VecExprOpt()( ( v2 + v3 )( 2 ) )";
    std::cout << " matches to VecExprOpt rule ..." << std::endl;
    syntaxChecker( LinAlgExprTrans()( ( v2 + v3 )( 2 ) ) );

    // Look ma, no temporaries!
    double d1 = trans( LinAlgExprTrans()( ( v2 + v3 )( 2 ) ) );
    std::cout << "( v2 + v3 )( 2 ) = " << d1 << std::endl;

    // Subtract two vectors and add the result to a third vector.
    v1 += v2 - v3;                  // Still no temporaries!
    std::cout << "v1 += v2 - v3" << std::endl;
    std::cout << "v1 =";
    std::cout << '{' << v1(0) << ',' << v1(1)
              << ',' << v1(2) << ',' << v1(3) << '}' << std::endl;


	Matrix m1( 2, 2), m2( 2, 2), m3( 2, 2);

	m1(0,0) = 1.00;
	m1(0,1) = 1.01;
	m1(1,0) = 1.10;
	m1(1,1) = 1.11;

	m2(0,0) = 2.00;
	m2(0,1) = 2.01;
	m2(1,0) = 2.10;
	m2(1,1) = 2.11;

	std::cout << "Checking if m1 + m2 matches to MatExprTrans rule ..."
			<< std::endl;
	syntaxChecker( m1 + m2 );

	std::cout << "Checking if (m1 + m2)(0,1) matches to MatExprTrans rule ..."
			<< std::endl;
	syntaxChecker( (m1 + m2)(0,1) );

	double elm01 = trans( LinAlgExprTrans()( (m1 + m2)(0,1) ) );
	std::cout << " (m1 + m2)(0,1) = " << elm01 << std::endl;

	m3 = m1 + m2;
	std::cout << "m3(0,0) = " << m3(0,0) << std::endl;
	std::cout << "m3(0,1) = " << m3(0,1) << std::endl;
	std::cout << "m3(1,0) = " << m3(1,0) << std::endl;
	std::cout << "m3(1,1) = " << m3(1,1) << std::endl;


    return 0;
}

