/*
 * DenseLinAlg.hpp
 *
 *  Created on: 2015/10/20
 *      Author: Masakatsu ITO
 */

#ifndef DENSELINALG_DENSELINALG_HPP_
#define DENSELINALG_DENSELINALG_HPP_

#include <iostream>
#include <boost/proto/proto.hpp>

#include <DenseLinAlg/Grammar.hpp>
#include <DenseLinAlg/MatrixVector.hpp>
#include <DenseLinAlg/LazyEvaluator.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


namespace DenseLinAlg {

	// Define a trait for detecting linear algebraic terminals, to be used
	// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
	template<typename> struct IsExpr  : mpl::false_ {};

	template<> struct IsExpr< Vector > : mpl::true_  {};
	template<> struct IsExpr< Matrix > : mpl::true_  {};
	template<> struct IsExpr< DiagonalMatrix > : mpl::true_  {};

	template<> struct IsExpr< LazyMatVecMult > : mpl::true_  {};

	template<> struct IsExpr< LazyVectorMaker > : mpl::true_  {};
	template<> struct IsExpr< LazyMatrixMaker > : mpl::true_  {};
	template<> struct IsExpr< LazyDiagonalMatrixMaker > : mpl::true_  {};

	// This defines all the overloads to make expressions involving
	// Vector and Matrix objects to build Proto's expression templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsExpr, Domain)
	// BOOST_PROTO_DEFINE_OPERATORS(IsExpr, proto::default_domain)

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


#endif /* DENSELINALG_DENSELINALG_HPP_ */
