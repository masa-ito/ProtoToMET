/*
 * vecAddMetaOpenMP.hpp
 *
 *  Created on: 2016/05/15
 *      Author: Masakatsu ITO
 */

#ifndef VECADDMETAOPENMP_HPP_
#define VECADDMETAOPENMP_HPP_

#include <iostream>
#include <boost/proto/proto.hpp>

#include <ParallelizationTypeTag/Default.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

namespace PTT = ParallelizationTypeTag;

namespace DenseLinAlg {

class Vector;

// The transformation rule for vector element expressions
// This transform accepts a subscript index  of an expression being parsed
// as the state variable,and distribute that index over the child nodes.
struct VecElmTrans : proto::or_<
	proto::when< proto::terminal< Vector>,
				proto::_make_function( proto::_, proto::_state) >,
	proto::plus< VecElmTrans, VecElmTrans> ,
	proto::minus< VecElmTrans, VecElmTrans>
> {};

// The tranformation rule for vector expressions
struct VecExprTrans : proto::or_<
	proto::when<
		proto::function< VecElmTrans, proto::_ >,
		proto::_default< >( VecElmTrans(proto::_left, proto::_right) )
	>,
	proto::terminal< Vector >,
	proto::plus< VecExprTrans, VecExprTrans> ,
	proto::minus< VecExprTrans, VecExprTrans>
> {};

// The tranformation rule for linear algebraic expressions
struct LinAlgExprTrans : proto::or_<
	VecExprTrans //,
	// MatExprTrans
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
		std::cout << "Copied" << std::endl;
	}

	~Vector() {
		delete [] data;
		std::cout << "Deleted" << std::endl;
	}

	// accesing to a vector element
	double& operator()(int i) { return data[i]; }
	const double& operator()(int i) const { return data[i]; }

	template < typename Expr >
	void assign( const Expr& expr ,
		const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& ) {
		for(int i=0; i < sz; ++i)
			data[i] = VecExprTrans()( expr(i) );
	}

	template < typename Expr >
	void assign( const Expr& expr ,
			const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) {
		#pragma omp parallel for
		for(int i=0; i < sz; ++i)
			data[i] = VecExprTrans()( expr(i) );

		std::cout << "OpenMP assign" << std::endl;
	}


	// assigning the lhs of a vector expression into this vector
	template<typename Expr>
	Vector& operator=( const Expr& expr ) {
		assign( expr, PTT::Specified() );
		return *this;
	}

	template < typename Expr >
	void plusAssign( const Expr& expr ,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& ) {
		for(int i=0; i < sz; ++i)
			data[i] += VecExprTrans()( expr(i) );
	}

	template < typename Expr >
	void plusAssign( const Expr& expr ,
				 const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) {
		#pragma omp parallel for
		for(int i=0; i < sz; ++i)
			data[i] += VecExprTrans()( expr(i) );

		std::cout << "OpenMP plusAssign" << std::endl;
	}

	// assigning and adding the lhs of a vector expression into this vector
	template<typename Expr>
	Vector& operator+=( const Expr& expr ) {
		plusAssign( expr, PTT::Specified() );
		return *this;
	}
};



// Define a trait for detecting linear algebraic terminals, to be used
// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
template<typename> struct IsLinAlg  : mpl::false_ {};
template<> struct IsLinAlg< Vector > : mpl::true_  {};
// template<> struct IsLinAlg< Matrix > : mpl::true_  {};


// This defines all the overloads to make expressions involving
// Vector and Matrix objects to build linear algebraic expression
// templates.
BOOST_PROTO_DEFINE_OPERATORS(IsLinAlg, LinAlgDomain)


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


}


namespace DLA = DenseLinAlg;

void testVecAdd()
{

	DLA::ExpressionSyntaxChecker< DLA::LinAlgExprTrans >
		syntaxChecker =
				DLA::ExpressionSyntaxChecker< DLA::LinAlgExprTrans >();
    // proto::_default<> trans;

    // lazy_vectors with 3 elements each.
    DLA::Vector v1( 3, 1.0 ), v2( 3, 2.0 ), v3( 3, 3.0 );

    // Add two vectors lazily and get the 2nd element.
    std::cout << "Checking if v2 + v3 matches to LinAlgExprTrans rule ..."
    		<< std::endl;
    syntaxChecker( v2 + v3 );

    std::cout << "Checking if (v2 + v3)[2] matches to LinAlgExprTrans rule ..."
    		<< std::endl;
    syntaxChecker( ( v2 + v3 )( 2 ) );

    /* std::cout << "Checking if VecExprOpt()( ( v2 + v3 )( 2 ) )";
    std::cout << " matches to VecExprOpt rule ..." << std::endl;
    syntaxChecker( LinAlgExprTrans()( ( v2 + v3 )( 2 ) ) ); */

    // Look ma, no temporaries!
    double d1 = DLA::LinAlgExprTrans()( ( v2 + v3 )( 2 ) );
    // double d1 = trans( LinAlgExprTrans()( ( v2 + v3 )( 2 ) ) );
    std::cout << "DLA::LinAlgExprTrans()( ( v2 + v3 )( 2 ) ) = " <<
    		d1 << std::endl;

    // Subtract two vectors and add the result to a third vector.
    v1 += v2 - v3;                  // Still no temporaries!
    std::cout << "v1 += v2 - v3" << std::endl;
    std::cout << "v1 =";
    std::cout << '{' << v1(0) << ',' << v1(1)
              << ',' << v1(2) << ',' << v1(3) << '}' << std::endl;

    return;
}




#endif /* VECADDMETAOPENMP_HPP_ */
