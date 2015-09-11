/*
 * transformingVectorOnHeapArray.cpp
 *
 *  Created on: 2015/07/16
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


class Vector;

struct IndDist : proto::or_<
	proto::when< proto::terminal< Vector>,
				proto::_make_subscript( proto::_, proto::_state) >,
	proto::terminal< Vector >,
	proto::plus< IndDist, IndDist> ,
	proto::minus< IndDist, IndDist>
> {};

// This grammar describes which vector expressions
// are allowed.
struct VecExprOpt : proto::or_<
	proto::when< proto::subscript< IndDist, proto::terminal< Vector> >,
				IndDist(proto::_left, proto::_right) >,
	proto::terminal< Vector >,
	proto::plus< VecExprOpt, VecExprOpt> ,
	proto::minus< VecExprOpt, VecExprOpt>
> {};


// The above grammar is associated with this domain.
template<typename Expr> struct VecExpr;
struct VecDomain
	: proto::domain<proto::generator<VecExpr>, VecExprOpt> {};


//
// Vector Expression Templates
//
template<typename Expr>
struct VecExpr
	: proto::extends<Expr, VecExpr<Expr>, VecDomain> {
		explicit VecExpr(const Expr& e)
			: proto::extends<Expr, VecExpr<Expr>, VecDomain>(e) {
		}

		const double& operator [](int i) const {
			proto::_default<> trans;
			std::cout << "const double& VecExpr<Expr>::operator [](int i) const used." << std::endl;
			return trans( VecExprOpt()(  proto::as_expr<VecDomain>( (*this)[i] )  ) );
			// return (*this)[i];
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

	// assigning the lhs of a vector expression into this vector
	template<typename Expr>
	Vector& operator=( const Expr& expr ) {
		proto::_default<> trans;
		for(int i=0; i < sz; ++i)
			data[i] = trans( VecExprOpt()( expr[i] ) );
		return *this;
	}

	// assigning and adding the lhs of a vector expression into this vector
	template<typename Expr>
	Vector& operator+=( const Expr& expr ) {
		proto::_default<> trans;
		for(int i=0; i < sz; ++i)
			data[i] += expr[i];
			//data[i] += trans( VecExprOpt()( expr[i] ) );
		std::cout << "Vector& operator+= done." << std::endl;
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
    // Checking if code optimization works properly.
    proto::display_expr( v2 + v3 );
    proto::display_expr( proto::as_expr<VecDomain>( ( v2 + v3 )[ 2 ] ) );
    // double d1 = ( v2 + v3 )[ 2 ];   // Look ma, no temporaries!
    //double d1 = v2[2] + v3[2];   // Look ma, no temporaries!
    // std::cout << d1 << std::endl;

    // Subtract two vectors and add the result to a third vector.
    /* v1 += v2 - v3;                  // Still no temporaries!
    std::cout << '{' << v1[0] << ',' << v1[1]
              << ',' << v1[2] << ',' << v1[3] << '}' << std::endl; */

    // This expression is disallowed because it does not conform
    // to the LazyVectorGrammar
    //(v2 + v3) += v1;

    return 0;
}


