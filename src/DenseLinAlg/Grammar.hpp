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
	class DiagonalMatrix;


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
		// Vector * double , or double * Vector
		proto::when<
			proto::multiplies< proto::terminal< Vector > ,
							proto::terminal< double > >,
			proto::_make_multiplies(
				proto::_make_function( proto::_left, proto::_state),
				proto::_right )
		>,
		proto::when<
			proto::multiplies< proto::terminal< double >,
							proto::terminal< Vector > >,
			proto::_make_multiplies(
				proto::_left,
				proto::_make_function( proto::_right, proto::_state) )
		>,
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
		// Vector * double , or double * Vector
		proto::multiplies< proto::terminal< Vector > ,
						proto::terminal< double > >,
		proto::multiplies< proto::terminal< double >,
						proto::terminal< Vector > >,
		// Matrix * Vector
		MatVecMultGrammar,
		// DiagonalMatrix * VecExprGrammar
		proto::multiplies< proto::terminal< DiagonalMatrix >,
						VecExprGrammar >
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

	// The transformation rule for diagonal matrix element expressions
	// with two indices
	struct DiagMatElmTwoIdxGrammar : proto::or_<
		// DiagonalMatrix
		proto::when< proto::terminal< DiagonalMatrix>,
					proto::_make_function( proto::_,
											proto::_state, proto::_data) >,
		// DiagMatElmTwoIdxGrammar +(-) DiagMatElmTwoIdxGrammar
		proto::plus< DiagMatElmTwoIdxGrammar, DiagMatElmTwoIdxGrammar >,
		proto::minus< DiagMatElmTwoIdxGrammar, DiagMatElmTwoIdxGrammar >
	> {};

	// The transformation rule for diagonal matrix element expressions
	// with one index
	struct DiagMatElmOneIdxGrammar : proto::or_<
		// DiagonalMatrix
		proto::when< proto::terminal< DiagonalMatrix>,
					proto::_make_function( proto::_, proto::_state) >,
		// DiagMatElmOneIdxGrammar +(-) DiagMatElmOneIdxGrammar
		proto::plus< DiagMatElmOneIdxGrammar, DiagMatElmOneIdxGrammar >,
		proto::minus< DiagMatElmOneIdxGrammar, DiagMatElmOneIdxGrammar >
	> {};

	// The tranformation rule for diagonal matrix expressions
	struct DiagMatExprGrammar : proto::or_<
		// DiagMatElmTwoIdxGrammar( rowIndex, columnIndex )
		proto::when<
			proto::function< DiagMatElmTwoIdxGrammar, proto::_, proto::_ >,
			DiagMatElmTwoIdxGrammar( proto::_child0,
									proto::_child1, proto::_child2)
		>,
		// DiagMatElmOneIdxGrammar( index )
		proto::when<
			proto::function< DiagMatElmOneIdxGrammar, proto::_ >,
			DiagMatElmTwoIdxGrammar( proto::_child0, proto::_child1)
		>,
		// DiagonalMatrix
		proto::terminal< DiagonalMatrix >,
		// DiagMatExprGrammar +(-) DiagMatExprGrammar
		proto::plus< DiagMatExprGrammar, DiagMatExprGrammar >,
		proto::minus< DiagMatExprGrammar, DiagMatExprGrammar>
	> {};

	// The tranformation rule for matrix expressions
	struct MatExprGrammar : proto::or_<
		// MatElmGrammar( rowIndex, columnIndex )
		proto::when<
				proto::function< MatElmGrammar, proto::_ , proto::_ >,
				MatElmGrammar(proto::_child0, proto::_child1, proto::_child2)
			>,
		// Matrix
		proto::terminal< Matrix >,
		// MatExprGrammar * MatExprGrammar
		proto::multiplies< MatExprGrammar, MatExprGrammar >,
		// MatExprGrammar * DiagMatExprGrammar
		proto::multiplies< MatExprGrammar, DiagMatExprGrammar >,
		// DigaMatExprGrammar * MatExprGrammar
		proto::multiplies< DiagMatExprGrammar, MatExprGrammar >,
		// MatExprGrammar +(-) MatExprGrammar
		proto::plus< MatExprGrammar, MatExprGrammar> ,
		proto::minus< MatExprGrammar, MatExprGrammar>
	> {};

	// The tranformation rule for linear algebraic expressions
	struct ExprGrammar : proto::or_<
		VecExprGrammar,
		MatExprGrammar,
		DiagMatExprGrammar
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
