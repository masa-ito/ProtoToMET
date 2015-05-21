/*
 * ProtofyingMyVectors.cpp
 *
 *  Created on: 2015/04/23
 *      Author: Masakatsu ITO
 *  Modification of a Boost.Proto example
 *  TArray: A Simple Linear Algebra Library
 */

///////////////////////////////////////////////////////////////////////////////
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// This example constructs a mini-library for linear algebra, using
// expression templates to eliminate the need for temporaries when
// adding arrays of numbers. It duplicates the TArray example from
// PETE (http://www.codesourcery.com/pooma/download.html)

#include <iostream>
#include <boost/format.hpp>
#include <boost/mpl/int.hpp>
#include <boost/proto/core.hpp>
#include <boost/proto/context.hpp>
namespace mpl = boost::mpl;
namespace proto = boost::proto;
using proto::_;

// This grammar describes which MyVector expressions
// are allowed; namely, int and array terminals
// plus, minus, multiplies and divides of MyVector expressions.
struct MyVectorGrammar
  : proto::or_<
  proto::terminal< std::vector<double> >
  , proto::plus< MyVectorGrammar, MyVectorGrammar >
  , proto::minus< MyVectorGrammar, MyVectorGrammar >
  >
{};

template<typename Expr>
struct MyVectorExpr;

// Tell proto that in the vector_domain, all
// expressions should be wrapped in MyVectorExpr<> and
// must conform to MyVectorGrammer .
struct MyVectorDomain
  : proto::domain<
    // use_basic_expr here instructs proto to use the stripped-
    // down proto::basic_expr instead of proto::expr to reduce
    // compile times. It's not necessary.
    proto::use_basic_expr<proto::pod_generator<MyVectorExpr> >
          , MyVectorGrammar
    >
{};

// Here is an evaluation context that indexes into a MyVector
// expression, and combines the result.
struct MyVectorIndexCtx
  : proto::callable_context< MyVectorIndexCtx const >
{
    typedef double result_type;

    MyVectorIndexCtx(std::size_t i)
      : i_(i)
    {}

    // Index array terminals with our index. Everything
    // else will be handled by the default evaluation context.
  double operator ()(proto::tag::terminal, std::vector<double> const &data) const
    {
        return data[this->i_];
    }

    std::size_t i_;
};


// Here is the domain-specific expression wrapper, which overrides
// operator [] to evaluate the expression using the MyVectorIndexCtx.
template<typename Expr>
struct MyVectorExpr
{
	  BOOST_PROTO_BASIC_EXTENDS(Expr, MyVectorExpr<Expr>, MyVectorDomain)

    // Use the MyVectorIndexCtx to implement indexing
    // of a MyVector expression tree.
    double operator []( std::size_t i ) const
    {
        MyVectorIndexCtx const ctx(i);
        return proto::eval(*this, ctx);
    }
};

// Here is our MyVector terminal, implemented in terms of MyVectorExpr
template<typename = proto::is_proto_expr>
struct MyVector_
  : MyVectorExpr< 
  proto::basic_expr< proto::tag::terminal,
		     proto::term< std::vector<double> > >
  >
{
	explicit MyVector_(std::size_t n =0) {
		 proto::value(*this).resize(n);
	 }

    // Here we override operator [] to give read/write access to
    // the elements of the array. (We could use the MyVectorExpr
    // operator [] if we made the index context smarter about
    // returning non-const reference when appropriate.)
    double &operator [](std::size_t i) {
        return proto::value(*this)[i];
    }

    double const &operator [](std::size_t i) const {
        return proto::value(*this)[i];
    }

    // Here we define a operator = for MyVector terminals that
    // takes a MyVector expression.
    template< typename Expr >
    MyVector_ &operator =(MyVectorExpr< Expr > const & rhs)
    {
    	std::size_t const size =
    			static_cast<std::size_t>(proto::value(*this).size());
    	for(std::size_t i = 0; i < size; ++i)
    		proto::value(*this)[i] = rhs[i];
    	return *this;
    }
};

typedef MyVector_<> MyVector;


int main()
{
	const int vecSz = 3;
    MyVector a( vecSz), b( vecSz), c( vecSz);
    for (std::size_t i=0; i < vecSz; i++)
    	b[i] = c[i] = i;

    a = b + c;


    std::cout << boost::format("a = {%d, %d, %d}") % a[0] %a[1] %a[2] ;
    std::cout << std::endl;

    return 0;
}


