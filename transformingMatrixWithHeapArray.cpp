/*
 * transformingMatrixWithHeapArray.cpp
 *
 *  Created on: 2015/09/28
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


class Matrix;

// The transformation rule for matrix element expressions
struct MatElmTrans : proto::or_<
	proto::when< proto::terminal< Matrix>,
				proto::_make_function( proto::_,
									proto::_state, proto::_data) >,
	proto::plus< MatElmTrans, MatElmTrans> ,
	proto::minus< MatElmTrans, MatElmTrans>
> {};

// The transformation rule for optimizing matrix expressions
struct MatExprTrans : proto::or_<
	proto::when<
		proto::function< MatElmTrans, proto::_ , proto::_ >,
		MatElmTrans(proto::_child0, proto::_child1, proto::_child2)
	>,
	proto::terminal< Matrix >,
	proto::plus< MatExprTrans, MatExprTrans> ,
	proto::minus< MatExprTrans, MatExprTrans>
> {};


// The above grammar is associated with this domain.
template<typename Expr> struct MatExpr;
struct MatDomain
	: proto::domain<proto::generator<MatExpr>, MatExprTrans> {};


//
// Matrix Expression Templates
//
template<typename Expr>
struct MatExpr
	: proto::extends<Expr, MatExpr<Expr>, MatDomain>
{
	explicit MatExpr(const Expr& e)
		: proto::extends<Expr, MatExpr<Expr>, MatDomain>(e)
	{}
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


// Define a trait for detecting vector terminals, to be used
// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
template<typename> struct IsMatrix  : mpl::false_ {};
template<> struct IsMatrix< Matrix> : mpl::true_  {};


namespace MatrixOps {
	// This defines all the overloads to make expressions involving
	// Matrix objects to build expression templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsMatrix, MatDomain)
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


int main() {
	using namespace MatrixOps;

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
	ExpressionSyntaxChecker< MatExprTrans>()( m1 + m2 );

	std::cout << "Checking if (m1 + m2)(0,1) matches to MatExprTrans rule ..."
			<< std::endl;
	ExpressionSyntaxChecker< MatExprTrans>()( (m1 + m2)(0,1) );

	proto::_default<> trans;
	double elm01 = trans( MatExprTrans()( (m1 + m2)(0,1) ) );
	std::cout << " (m1 + m2)(0,1) = " << elm01 << std::endl;

	m3 = m1 + m2;
	std::cout << "m3(0,0) = " << m3(0,0) << std::endl;
	std::cout << "m3(0,1) = " << m3(0,1) << std::endl;
	std::cout << "m3(1,0) = " << m3(1,0) << std::endl;
	std::cout << "m3(1,1) = " << m3(1,1) << std::endl;
}




