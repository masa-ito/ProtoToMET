/*
 * adaptingArrayWrapper.cpp
 *
 *  Created on: 2015/05/20
 *      Author: Masakatsu ITO
 */

// The original version of this program is
// an example in Boost.Proto users' guide
// "Vector: Adapting a Non-Proto Terminal Type"
//
///////////////////////////////////////////////////////////////////////////////
//  Copyright 2008 Eric Niebler. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// This is an example of using BOOST_PROTO_DEFINE_OPERATORS to Protofy
// expressions using std::vector<>, a non-proto type. It is a port of the
// Vector example from PETE (http://www.codesourcery.com/pooma/download.html).
//
// It was modified to adapt a primitive array instead of std::vector .

#include <vector>
#include <iostream>
#include <stdexcept>
#include <boost/mpl/bool.hpp>
#include <boost/proto/core.hpp>
#include <boost/proto/debug.hpp>
#include <boost/proto/context.hpp>
#include <boost/utility/enable_if.hpp>
namespace mpl = boost::mpl;
namespace proto = boost::proto;
using proto::_;


template <typename T>
class ArrayWrapper {
private:
	T* data;
	size_t size_;

public:
	typedef T value_type;

	explicit ArrayWrapper(std::size_t size = 0, T const & value = T() ):
		data( new T[size]), size_(size) {
    	for (std::size_t i = 0; i < size_; i++) data[i] = value;

	}
	ArrayWrapper(const ArrayWrapper<T>& wrapper):
		data( new T[ wrapper.size_] ), size_(wrapper.size_) {
		for (std::size_t i = 0; i < size_; i++) data[i] = wrapper.data[i];
	}

	~ArrayWrapper() {
		std::cerr << "Now destructing an ArrayWrapper" << std::endl;
		delete [] data;
	}

	std::size_t size() const {
		return size_;
	}

	T& operator[](std::size_t i) {
		return data[i];
	}
	T operator[](std::size_t i) const {
		return data[i];
	}
};


template<typename Expr>
struct VectorExpr;

// Here is an evaluation context that indexes into a std::vector
// expression and combines the result.
struct VectorSubscriptCtx
{
    VectorSubscriptCtx(std::size_t i)
      : i_(i)
    {}

    // Unless this is a vector terminal, use the
    // default evaluation context
    template<typename Expr, typename EnableIf = void>
    struct eval
      : proto::default_eval<Expr, VectorSubscriptCtx const>
    {};

    // Index vector terminals with our subscript.
    template<typename Expr>
    struct eval<
        Expr
      , typename boost::enable_if<
            proto::matches<Expr, proto::terminal<ArrayWrapper<_ > > >
        >::type
    >
    {
        typedef typename proto::result_of::value<Expr>::type::value_type result_type;

        result_type operator ()(Expr &expr, VectorSubscriptCtx const &ctx) const
        {
            return proto::value(expr)[ctx.i_];
        }
    };

    std::size_t i_;
};

// Here is an evaluation context that verifies that all the
// vectors in an expression have the same size.
struct VectorSizeCtx
{
    VectorSizeCtx(std::size_t size)
      : size_(size)
    {}

    // Unless this is a vector terminal, use the
    // null evaluation context
    template<typename Expr, typename EnableIf = void>
    struct eval
      : proto::null_eval<Expr, VectorSizeCtx const>
    {};

    // Index array terminals with our subscript. Everything
    // else will be handled by the default evaluation context.
    template<typename Expr>
    struct eval<
        Expr
      , typename boost::enable_if<
            proto::matches<Expr, proto::terminal<ArrayWrapper<_> > >
        >::type
    >
    {
        typedef void result_type;

        result_type operator ()(Expr &expr, VectorSizeCtx const &ctx) const
        {
            if(ctx.size_ != proto::value(expr).size())
            {
                throw std::runtime_error("LHS and RHS are not compatible");
            }
        }
    };

    std::size_t size_;
};

// A grammar which matches all the assignment operators,
// so we can easily disable them.
struct AssignOps
  : proto::switch_<struct AssignOpsCases>
{};

// Here are the cases used by the switch_ above.
struct AssignOpsCases
{
    template<typename Tag, int D = 0> struct case_  : proto::not_<_> {};

    template<int D> struct case_< proto::tag::plus_assign, D >         : _ {};
    template<int D> struct case_< proto::tag::minus_assign, D >        : _ {};
    template<int D> struct case_< proto::tag::multiplies_assign, D >   : _ {};
    template<int D> struct case_< proto::tag::divides_assign, D >      : _ {};
    template<int D> struct case_< proto::tag::modulus_assign, D >      : _ {};
    template<int D> struct case_< proto::tag::shift_left_assign, D >   : _ {};
    template<int D> struct case_< proto::tag::shift_right_assign, D >  : _ {};
    template<int D> struct case_< proto::tag::bitwise_and_assign, D >  : _ {};
    template<int D> struct case_< proto::tag::bitwise_or_assign, D >   : _ {};
    template<int D> struct case_< proto::tag::bitwise_xor_assign, D >  : _ {};
};

// A vector grammar is a terminal or some op that is not an
// assignment op. (Assignment will be handled specially.)
struct VectorGrammar
  : proto::or_<
        proto::terminal<_>
      , proto::and_<proto::nary_expr<_, proto::vararg<VectorGrammar> >, proto::not_<AssignOps> >
    >
{};

// Expressions in the vector domain will be wrapped in VectorExpr<>
// and must conform to the VectorGrammar
struct VectorDomain
  : proto::domain<proto::generator<VectorExpr>, VectorGrammar>
{};

// Here is VectorExpr, which extends a proto expr type by
// giving it an operator [] which uses the VectorSubscriptCtx
// to evaluate an expression with a given index.
template<typename Expr>
struct VectorExpr
  : proto::extends<Expr, VectorExpr<Expr>, VectorDomain>
{
    explicit VectorExpr(Expr const &expr)
      : proto::extends<Expr, VectorExpr<Expr>, VectorDomain>(expr)
    {}

    // Use the VectorSubscriptCtx to implement subscripting
    // of a Vector expression tree.
    typename proto::result_of::eval<Expr const, VectorSubscriptCtx const>::type
    operator []( std::size_t i ) const
    {
        VectorSubscriptCtx const ctx(i);
        return proto::eval(*this, ctx);
    }
};

// Define a trait type for detecting vector terminals, to
// be used by the BOOST_PROTO_DEFINE_OPERATORS macro below.
template<typename T>
struct IsVector
  : mpl::false_
{};

template<typename T>
struct IsVector<ArrayWrapper<T> >
  : mpl::true_
{};

namespace VectorOps
{
    // This defines all the overloads to make expressions involving
    // std::vector to build expression templates.
    BOOST_PROTO_DEFINE_OPERATORS(IsVector, VectorDomain)

    typedef VectorSubscriptCtx const CVectorSubscriptCtx;

    // Assign to a vector from some expression.
    template<typename T, typename Expr>
    ArrayWrapper<T> &assign(ArrayWrapper<T> &arr, Expr const &expr)
    {
        VectorSizeCtx const size(arr.size());
        proto::eval(proto::as_expr<VectorDomain>(expr), size); // will throw if the sizes don't match
        for(std::size_t i = 0; i < arr.size(); ++i)
        {
            arr[i] = proto::as_expr<VectorDomain>(expr)[i];
        }
        return arr;
    }

    // Add-assign to a vector from some expression.
    template<typename T, typename Expr>
    ArrayWrapper<T> &operator +=(ArrayWrapper<T> &arr, Expr const &expr)
    {
        VectorSizeCtx const size(arr.size());
        proto::eval(proto::as_expr<VectorDomain>(expr), size); // will throw if the sizes don't match
        for(std::size_t i = 0; i < arr.size(); ++i)
        {
            arr[i] += proto::as_expr<VectorDomain>(expr)[i];
        }
        return arr;
    }
}

int main()
{
    using namespace VectorOps;

    int i;
    const int n = 10;
    ArrayWrapper<int> a(n), b(n), c(n),d(n);
    ArrayWrapper<double> e(n);

    for (i = 0; i < n; ++i)
    {
    	a[i] = i;
    	b[i] = 2*i;
    	c[i] = 3*i;
    	d[i] = i;
    }

    VectorOps::assign(b, 2);
    VectorOps::assign(d, a + b * c);
    a += if_else(d < 30, b, c);

    VectorOps::assign(e, c);
    e += e - 4 / (c + 1);

    for (i = 0; i < n; ++i)
    {
        std::cout
            << " a(" << i << ") = " << a[i]
            << " b(" << i << ") = " << b[i]
            << " c(" << i << ") = " << c[i]
            << " d(" << i << ") = " << d[i]
            << " e(" << i << ") = " << e[i]
            << std::endl;
    }
}




