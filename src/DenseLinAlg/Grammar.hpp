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


namespace DenseLinAlg {

	namespace mpl = boost::mpl;
	namespace proto = boost::proto;

	class Vector;
	class Matrix;
	class DiagonalMatrix;


	// Callable transform object to make a proto exression
	// for lazily evaluating multiplication
	struct MatVecMult;
	struct MatVecMultOmp;

	// The grammar for the multiplication of a matrix and a vector
	struct MatVecMultGrammar : proto::or_<
		proto::when<
			proto::multiplies< proto::terminal< Matrix> ,
								proto::terminal< Vector> >,
			MatVecMult( proto::_value( proto::_left),
						proto::_value( proto::_right) )
		>
	> {};

	// For OpenMP
	struct MatVecMultOmpGrammar : proto::or_<
		proto::when<
			proto::multiplies< proto::terminal< Matrix > ,
								proto::terminal< Vector > >,
			MatVecMultOmp( proto::_value( proto::_left),
						proto::_value( proto::_right) )
		>
	> {};


	// The transformation rule for vector element expressions
	// This transform accepts a subscript index  of an expression being parsed
	// as the state variable,and distribute that index over the child nodes.

	// Elemenetwise type expression
	struct VecElementwiseElmGrammar : proto::or_<
		// Vector
		proto::when< proto::terminal< Vector >,
					proto::_make_function( proto::_, proto::_state) >,


		// Vector * double
		proto::when<
			proto::multiplies< proto::terminal< Vector > ,
							proto::terminal< double > >,
			proto::_make_multiplies(
				proto::_make_function( proto::_left, proto::_state),
				proto::_right
			)
		>,
		// double * Vector
		proto::when<
			proto::multiplies< proto::terminal< double >,
							proto::terminal< Vector > >,
			proto::_make_multiplies(
				proto::_left,
				proto::_make_function( proto::_right, proto::_state)
			)
		>,

		// DiagonalMatrix * Vector
		proto::when<
			proto::multiplies< proto::terminal< DiagonalMatrix >,
						   	   proto::terminal< Vector > >,
			proto::_make_multiplies(
				proto::_make_function( proto::_left, proto::_state),
				proto::_make_function( proto::_right, proto::_state)
			)
		>,

		// VecElementwiseElmGrammar +(-) VecElementwiseElmGrammar
		proto::plus< VecElementwiseElmGrammar, VecElementwiseElmGrammar > ,
		proto::minus< VecElementwiseElmGrammar, VecElementwiseElmGrammar >
	> {};


	// Reduction type expression
	struct VecReductionElmGrammar : proto::or_<
		// Matrix * Vector
		proto::when< MatVecMultGrammar,
					proto::_make_function( MatVecMultGrammar( proto::_),
											proto::_state) >,

		// VecReductionElmGrammar +(-) VecElementwiseElmGrammar
		proto::plus< VecReductionElmGrammar, VecElementwiseElmGrammar > ,
		proto::minus< VecReductionElmGrammar, VecElementwiseElmGrammar > ,

		// VecElementwiseElmGrammar +(-) VecReductionElmGrammar
		proto::plus< VecElementwiseElmGrammar, VecReductionElmGrammar > ,
		proto::minus< VecElementwiseElmGrammar, VecReductionElmGrammar >
	> {};

	// For OpenMP
	// Reduction type expression
	struct VecReductionElmOmpGrammar : proto::or_<
		// Matrix * Vector
		proto::when< MatVecMultOmpGrammar,
					proto::_make_function( MatVecMultOmpGrammar( proto::_),
											proto::_state) >,

		// VecReductionElmOmpGrammar +(-) VecReductionElmOmpGrammar
		proto::plus< VecReductionElmOmpGrammar,
					VecReductionElmOmpGrammar > ,
		proto::minus< VecReductionElmOmpGrammar,
					VecReductionElmOmpGrammar >,

		// VecReductionElmOmpGrammar +(-) VecElementwiseElmGrammar
		proto::plus< VecReductionElmOmpGrammar, VecElementwiseElmGrammar > ,
		proto::minus< VecReductionElmOmpGrammar, VecElementwiseElmGrammar > ,

		// VecElementwiseElmGrammar +(-) VecReductionElmOmpGrammar
		proto::plus< VecElementwiseElmGrammar, VecReductionElmOmpGrammar > ,
		proto::minus< VecElementwiseElmGrammar, VecReductionElmOmpGrammar >
	> {};


	// The grammar for a vector expression
	// Elementwise type expresson
	struct VecElementwiseGrammar : proto::or_<
		// VecElementwiseElmGrammar( index )
		proto::when<
			proto::function< VecElementwiseElmGrammar, proto::_ >,
			proto::_default< >(
				VecElementwiseElmGrammar(proto::_left, proto::_right) )
		>,

		// Vector
		proto::terminal< Vector >,

		// VecElementwiseGrammar +(-) VecElementwiseGrammar
		proto::plus< VecElementwiseGrammar, VecElementwiseGrammar > ,
		proto::minus< VecElementwiseGrammar, VecElementwiseGrammar >,

		// Vector * double , or double * Vector
		proto::multiplies< proto::terminal< Vector > ,
						proto::terminal< double > >,
		proto::multiplies< proto::terminal< double >,
						proto::terminal< Vector > >,

		// DiagonalMatrix * VecExprGrammar
		proto::multiplies< proto::terminal< DiagonalMatrix >,
							VecElementwiseGrammar >
	> {};


	// Reduction type expression
	struct VecReductionGrammar : proto::or_<
		// VecReductionElmGrammar( index )
		proto::when<
			proto::function< VecReductionElmGrammar, proto::_ >,
			proto::_default< >(
				VecReductionElmGrammar(proto::_left, proto::_right) )
		>,

		// VecReductionGrammar +(-) VecReductionGrammar
		proto::plus< VecReductionGrammar, VecReductionGrammar > ,
		proto::minus< VecReductionGrammar, VecReductionGrammar >,

		// VecReductionGrammar +(-) VecElementwiseGrammar
		proto::plus< VecReductionGrammar, VecElementwiseGrammar > ,
		proto::minus< VecReductionGrammar, VecElementwiseGrammar >,

		// VecElementwiseGrammar +(-) VecReductionGrammar
		proto::plus< VecElementwiseGrammar, VecReductionGrammar > ,
		proto::minus< VecElementwiseGrammar, VecReductionGrammar >,

		// Matrix * Vector
		MatVecMultGrammar //,
	> {};

	// For OpenMP
	// Reduction type expression
	struct VecReductionOmpGrammar : proto::or_<
		// VecReductionElmOmpGrammar( index )
		proto::when<
			proto::function< VecReductionElmOmpGrammar, proto::_ >,
			proto::_default< >(
				VecReductionElmOmpGrammar(proto::_left, proto::_right) )
		>,

		// VecReductionOmpGrammar +(-) VecReductionOmpGrammar
		proto::plus< VecReductionOmpGrammar, VecReductionOmpGrammar > ,
		proto::minus< VecReductionOmpGrammar, VecReductionOmpGrammar >,

		// VecReductionOmpGrammar +(-) VecElementwiseGrammar
		proto::plus< VecReductionOmpGrammar, VecElementwiseGrammar > ,
		proto::minus< VecReductionOmpGrammar, VecElementwiseGrammar >,

		// VecElementwiseGrammar +(-) VecReductionOmpGrammar
		proto::plus< VecElementwiseGrammar, VecReductionOmpGrammar > ,
		proto::minus< VecElementwiseGrammar, VecReductionOmpGrammar >,

		// Matrix * Vector
		MatVecMultOmpGrammar //,
	> {};


	// The grammar for a vector expression
	struct VecExprGrammar : proto::or_<
		VecElementwiseGrammar,
		VecReductionGrammar,
		VecReductionOmpGrammar
	> {};


	struct MatDiagmatMatMult;
	// struct LazyMatDiagmatMatMult;

	// The transformation rule for matrix element expressions
	struct MatElmGrammar : proto::or_<
		// Matrix
		proto::when<
			proto::terminal< Matrix >,
			proto::_make_function( proto::_,
										proto::_state, proto::_data)
		>,

		// Matrix * DiagonalMatrix * Matrix
		proto::when<
			proto::multiplies<
				proto::multiplies<
					proto::terminal< Matrix >,
					proto::terminal< DiagonalMatrix >
				>,
				proto::terminal< Matrix >
			>,
			proto::_make_function(
				MatDiagmatMatMult(
						proto::_value( proto::_left( proto::_left ) ),
						proto::_value( proto::_right( proto::_left) ),
						proto::_value( proto::_right )
				),
				proto::_state, proto::_data
			)
		>,

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
			proto::_default< >(
					DiagMatElmTwoIdxGrammar( proto::_child0,
									proto::_child1, proto::_child2)
			)
		>,
		// DiagMatElmOneIdxGrammar( index )
		proto::when<
			proto::function< DiagMatElmOneIdxGrammar, proto::_ >,
			proto::_default< >(
					DiagMatElmTwoIdxGrammar( proto::_child0, proto::_child1)
			)
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
				proto::_default< >(
						MatElmGrammar(proto::_child0,
								proto::_child1, proto::_child2)
				)
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
