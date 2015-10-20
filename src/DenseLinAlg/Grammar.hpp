/*
 * Grammar.hpp
 *
 *  Created on: 2015/10/20
 *      Author: Masakatsu ITO
 */

#ifndef DENSELINALG_GRAMMAR_HPP_
#define DENSELINALG_GRAMMAR_HPP_

#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


namespace DenseLinAlg {

	class Vector;
	class Matrix;


	// Callable transform object to make a proto exression
	// for lazily evaluationg a. multiplication
	struct MatVecMult;

	// The grammar for the multiplication of a matrix and a vector
	struct MatVecMultGrammar : proto::or_<
		proto::when<
			proto::multiplies< proto::terminal< Matrix> ,
								proto::terminal< Vector> >,
			MatVecMult( proto::_value( proto::_left),
						proto::_value( proto::_right) )
		>
	> {};

	// The transformation rule for vector element expressions
	// This transform accepts a subscript index  of an expression being parsed
	// as the state variable,and distribute that index over the child nodes.
	struct VecElmGrammar : proto::or_<
		// Vector
		proto::when< proto::terminal< Vector>,
					proto::_make_function( proto::_, proto::_state) >,
		// Matrix * Vector
		proto::when< MatVecMultGrammar,
					proto::_make_function( MatVecMultGrammar( proto::_),
											proto::_state) >,
		// VecElmGrammar +(-) VecElmGrammar
		proto::plus< VecElmGrammar, VecElmGrammar> ,
		proto::minus< VecElmGrammar, VecElmGrammar>
	> {};

	// The grammar for a vector expression
	struct VecExprGrammar : proto::or_<
		// VecElmGrammar( index )
		proto::when< proto::function< VecElmGrammar, proto::_ >,
					VecElmGrammar(proto::_left, proto::_right) >,
		// Vector
		proto::terminal< Vector >,
		// VecExprGrammar +(-) VecExprGrammar
		proto::plus< VecExprGrammar, VecExprGrammar> ,
		proto::minus< VecExprGrammar, VecExprGrammar>,
		// Matrix * Vector
		MatVecMultGrammar
	> {};

	// The transformation rule for matrix element expressions
	struct MatElmGrammar : proto::or_<
		// Matrix
		proto::when< proto::terminal< Matrix>,
					proto::_make_function( proto::_,
										proto::_state, proto::_data) >,
		// MatElmGrammar +(-) MatElmGrammar
		proto::plus< MatElmGrammar, MatElmGrammar> ,
		proto::minus< MatElmGrammar, MatElmGrammar>
	> {};

	// The tranformation rule for matrix expressions
	struct MatExprGrammar : proto::or_<
		// MatElmGrammar( rowIndex, columnIndex )
		proto::when<
				proto::function< MatElmGrammar, proto::_ , proto::_ >,
				MatElmGrammar(proto::_child0, proto::_child1, proto::_child2)
			>,
		// matrix
		proto::terminal< Matrix >,
		// MatExprGrammar +(-) MatExprGrammar
		proto::plus< MatExprGrammar, MatExprGrammar> ,
		proto::minus< MatExprGrammar, MatExprGrammar>
	> {};

	// The tranformation rule for linear algebraic expressions
	struct ExprGrammar : proto::or_<
		VecExprGrammar,
		MatExprGrammar
	> {};


	template <typename GrammarType>
	struct GrammarChecker
	{
		template <class Expr>
		void operator ()(Expr const & expr) const {
			static_assert(
					proto::matches<Expr, GrammarType>::value,
					"The expression does not match to the grammar!"
			);
			proto::display_expr( expr );
			std::cout << std::endl;
		}
	};


}




#endif /* DENSELINALG_GRAMMAR_HPP_ */
