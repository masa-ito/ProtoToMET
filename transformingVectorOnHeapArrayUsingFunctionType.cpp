/*
 * transformingVectorOnHeapArrayUsingFunctionType.cpp
 *
 *  Created on: 2015/07/16
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <type_traits>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


class Vector;

// This transform accepts a subscript index  of an expression being parsed
// as the state variable,and distribute that index over the child nodes.
struct VecElmTrans : proto::or_<
	proto::when< proto::terminal< Vector>,
				proto::_make_function( proto::_, proto::_state) >,
	proto::plus< VecElmTrans, VecElmTrans> ,
	proto::minus< VecElmTrans, VecElmTrans>
> {};

// This grammar describes which vector expressions
// are allowed.
struct VecExprTrans : proto::or_<
	proto::when< proto::function< VecElmTrans, proto::_ >,
				VecElmTrans(proto::_left, proto::_right) >,
	proto::terminal< Vector >,
	proto::plus< VecExprTrans, VecExprTrans> ,
	proto::minus< VecExprTrans, VecExprTrans>
> {};


// The above grammar is associated with this domain.
template<typename Expr> struct VecExpr;
struct VecDomain
	: proto::domain<proto::generator<VecExpr>, VecExprTrans> {};


//
// Vector Expression Templates
//
template<typename Expr>
struct VecExpr
	: proto::extends<Expr, VecExpr<Expr>, VecDomain> {
		explicit VecExpr(const Expr& e)
			: proto::extends<Expr, VecExpr<Expr>, VecDomain>(e) {
		}
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


// Define a trait for detecting vector terminals, to be used
// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
template<typename> struct IsVector : mpl::false_ {};
template<> struct IsVector<Vector> : mpl::true_  {};


namespace VectorOps {
	// This defines all the overloads to make expressions involving
	// Vector objects to build expression templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsVector, VecDomain)
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
	using namespace VectorOps;

    // lazy_vectors with 4 elements each.
    Vector v1( 4, 1.0 ), v2( 4, 2.0 ), v3( 4, 3.0 );

    // Add two vectors lazily and get the 2nd element.
    std::cout << "Checking if v2 + v3 matches to VecExprOpt rule ..."
    		<< std::endl;
    ExpressionSyntaxChecker< VecExprTrans >()( v2 + v3 );

    std::cout << "Checking if (v2 + v3)[2] matches to VecExprOpt rule ..."
    		<< std::endl;
    ExpressionSyntaxChecker< VecExprTrans >()( ( v2 + v3 )( 2 ) );

    std::cout << "Checking if VecExprOpt()( ( v2 + v3 )( 2 ) )";
    std::cout << " matches to VecExprOpt rule ..." << std::endl;
    ExpressionSyntaxChecker< VecExprTrans >()( VecExprTrans()( ( v2 + v3 )( 2 ) ) );

    proto::_default<> trans;
    // Look ma, no temporaries!
    double d1 = trans( VecExprTrans()( ( v2 + v3 )( 2 ) ) );
    std::cout << "( v2 + v3 )( 2 ) = " << d1 << std::endl;

    // Subtract two vectors and add the result to a third vector.
    v1 += v2 - v3;                  // Still no temporaries!
    std::cout << "v1 += v2 - v3" << std::endl;
    std::cout << "v1 =";
    std::cout << '{' << v1(0) << ',' << v1(1)
              << ',' << v1(2) << ',' << v1(3) << '}' << std::endl;

    // This expression is disallowed because it does not conform
    // to the LazyVectorGrammar
    //(v2 + v3) += v1;

    return 0;
}


