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
	VecExprTrans
	// MatExprTrans
> {};


// Callable transform object
// for lazily assigning a vector expression into a vector
struct AssignVecExpr;
struct PlusAssignVecExpr;

// The transformation rule for assigning a vector expression
// into a vector object
struct AssignVecExprTrans : proto::when<
	VecExprTrans,
	AssignVecExpr( proto::_, proto::_state, proto::_data)
> {};

struct PlusAssignVecExprTrans : proto::when<
	VecExprTrans,
	PlusAssignVecExpr( proto::_, proto::_state, proto::_data)
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

	// assigning the lhs of a vector expression into this vector
	template < typename Expr >
	Vector& operator=( const Expr& expr ) {
		AssignVecExprTrans()( expr, *this, PTT::Specified());
		return *this;
	}

	// assigning and adding the lhs of a vector expression into this vector
	template < typename Expr >
	Vector& operator+=( const Expr& expr ) {
		PlusAssignVecExprTrans()( expr, *this, PTT::Specified() );
		return *this;
	}

	friend class AssignVecExpr;
	friend class PlusAssignVecExpr;
};


// Proto primitive transform object for implemeting
// operator=(Vector&, {vector expression})
struct AssignVecExpr : proto::callable
{
	typedef void result_type;

	template < typename Expr >
	result_type
	operator()( const Expr& expr, Vector& rhs,
		const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& ) const
	{
		for(int i=0; i < rhs.sz; ++i)
				rhs.data[i] = VecExprTrans()( expr(i) );
		return;
	}

	template < typename Expr >
	result_type
	operator()( const Expr& expr, Vector& rhs,
			const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) const
	{
		#pragma omp parallel for
		for(int i=0; i < rhs.sz; ++i)
				rhs.data[i] = VecExprTrans()( expr(i) );
		return;
	}
};

// Proto primitive transform object for implemeting
// operator+=(Vector&, {vector expression})
struct PlusAssignVecExpr : proto::callable
{
	typedef void result_type;

	template < typename Expr >
	result_type
	operator()( const Expr& expr, Vector& rhs,
		const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& ) const
	{
		for(int i=0; i < rhs.sz; ++i)
				rhs.data[i] += VecExprTrans()( expr(i) );
		return;
	}

	template < typename Expr >
	result_type
	operator()( const Expr& expr, Vector& rhs,
			const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) const
	{
		#pragma omp parallel for
		for(int i=0; i < rhs.sz; ++i)
				rhs.data[i] += VecExprTrans()( expr(i) );
		return;
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



#endif /* VECADDMETAOPENMP_HPP_ */
