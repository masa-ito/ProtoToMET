/*
 * adaptingVectorWithHeapArray.cpp
 *
 *  Created on: 2015/05/29
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

// This grammar describes which vector expressions
// are allowed; namely, vector terminals and addition
// and subtraction of vector expressions.
struct VecGrammar : proto::or_<
	proto::terminal< proto::_ >,
	proto::plus< VecGrammar, VecGrammar>,
	proto::minus< VecGrammar, VecGrammar>
> {};


// The above grammar is associated with this domain.
template<typename Expr> struct VecExpr;
struct VecDomain
	: proto::domain<proto::generator<VecExpr>, VecGrammar> {};


//
// Context for evaluating an element of matrix expressions
//
struct SubscriptCntxt
	: proto::callable_context<const SubscriptCntxt> {
		typedef double result_type;

		int index;
		SubscriptCntxt(int index_) :  index(index_) {}

		// matrix element
		template<typename Vector>
		double operator()(proto::tag::terminal, const Vector& vec) const {
			return vec[index];
		}

		// addition of vector expression terms
		template<typename E1, typename E2>
		double operator()(proto::tag::plus, const E1& e1, const E2& e2) const {
			return proto::eval(e1, *this) + proto::eval(e2, *this);
		}

		// substraction of vector expression terms
		template<typename E1, typename E2>
		double operator()(proto::tag::minus, const E1& e1, const E2& e2) const {
			return proto::eval(e1, *this) - proto::eval(e2, *this);
		}
};


//
// Vector Expression Templates
//
template<typename Expr>
struct VecExpr
	: proto::extends<Expr, VecExpr<Expr>, VecDomain> {
		explicit VecExpr(const Expr& e)
			: proto::extends<Expr, VecExpr<Expr>, VecDomain>(e) {
		}

		// Use a SubscriptCntxt instance to implement subscripting
		// of a vector expression tree.
		typename proto::result_of::eval< Expr, SubscriptCntxt>::type
		operator [](int i) const {
			const SubscriptCntxt ctx(i);
			return proto::eval(*this, ctx);
		}
};

//
// Matrix data are stored in an heap array.
//
class Vector {
	private:
		int sz;
		double* data;

public:
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
	double& operator[](int i) { return data[i]; }
	const double& operator[](int i) const { return data[i]; }

	// assigning the lhs of a vector expression into this matrix
	template<typename Expr>
	Vector& operator=( const Expr& expr ) {
		for(int i=0; i < sz; ++i) {
				// evaluating the i'th element of a matrix expression
				const SubscriptCntxt ctx(i);
				data[i] = proto::eval(proto::as_expr<VecDomain>(expr), ctx);
		}
		return *this;
	}

	// assigning and adding the lhs of a vector expression into this matrix
	template<typename Expr>
	Vector& operator+=( const Expr& expr ) {
		for(int i=0; i < sz; ++i) {
				// evaluating the (i,j) element of a matrix expression
				const SubscriptCntxt ctx(i);
				data[i] += proto::eval(proto::as_expr<VecDomain>(expr), ctx);
		}
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

int main()
{
	using namespace VectorOps;

    // lazy_vectors with 4 elements each.
    Vector v1( 4, 1.0 ), v2( 4, 2.0 ), v3( 4, 3.0 );

    // Add two vectors lazily and get the 2nd element.
    double d1 = ( v2 + v3 )[ 2 ];   // Look ma, no temporaries!
    std::cout << d1 << std::endl;

    // Subtract two vectors and add the result to a third vector.
    v1 += v2 - v3;                  // Still no temporaries!
    std::cout << '{' << v1[0] << ',' << v1[1]
              << ',' << v1[2] << ',' << v1[3] << '}' << std::endl;

    // This expression is disallowed because it does not conform
    // to the LazyVectorGrammar
    //(v2 + v3) += v1;

    return 0;
}


