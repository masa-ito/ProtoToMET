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

#include <ParallelizationTypeTag/ParallelizationTypeTag.hpp>


namespace DenseLinAlg {

	namespace PAR = ParallelizationTypeTag;

	template < typename MultithreadingType > class Vector;
	template < typename MultithreadingType > class Matrix;
	template < typename MultithreadingType > class DiagonalMatrix;


	// Callable transform object to make a proto exression
	// for lazily evaluationg a. multiplication
	template < typename MultithreadingType > struct MatVecMult;

	// The grammar for the multiplication of a matrix and a vector
	template < typename MultithreadingType >
	struct MatVecMultGrammar : proto::or_<
		proto::when<
			proto::multiplies<
				proto::terminal< Matrix< MultithreadingType > > ,
				proto::terminal< Vector< MultithreadingType > >
			>,
			MatVecMult< MultithreadingType >(
						proto::_value( proto::_left),
						proto::_value( proto::_right)
			)
		>
	> {};


	// The transformation rule for vector element expressions
	// This transform accepts a subscript index  of an expression being parsed
	// as the state variable,and distribute that index over the child nodes.
	template < typename MultithreadingType >
	struct VecElmGrammar : proto::or_<
		// Vector
		proto::when< proto::terminal< Vector< MultithreadingType > >,
					proto::_make_function( proto::_, proto::_state) >,
		// Matrix * Vector
		proto::when<
			MatVecMultGrammar< MultithreadingType >,
			proto::_make_function(
				MatVecMultGrammar< MultithreadingType >( proto::_),
				proto::_state
			)
		>,
		// Vector * double
		proto::when<
			proto::multiplies<
				proto::terminal< Vector< MultithreadingType > > ,
				proto::terminal< double >
			>,
			proto::_make_multiplies(
				proto::_make_function( proto::_left, proto::_state),
				proto::_right
			)
		>,
		// double * Vector
		proto::when<
			proto::multiplies< proto::terminal< double >,
							proto::terminal< Vector< MultithreadingType > > >,
			proto::_make_multiplies(
				proto::_left,
				proto::_make_function( proto::_right, proto::_state)
			)
		>,
		// DiagonalMatrix * Vector
		proto::when<
			proto::multiplies<
				proto::terminal< DiagonalMatrix< MultithreadingType > >,
				proto::terminal< Vector< MultithreadingType > >
			>,
			proto::_make_multiplies(
				proto::_make_function( proto::_left, proto::_state),
				proto::_make_function( proto::_right, proto::_state)
			)
		>,
		// VecElmGrammar +(-) VecElmGrammar
		proto::plus< VecElmGrammar< MultithreadingType >,
					VecElmGrammar< MultithreadingType > > ,
		proto::minus< VecElmGrammar< MultithreadingType >,
					VecElmGrammar< MultithreadingType > >
	> {};


	// The grammar for a vector expression
	template < typename MultithreadingType >
	struct VecExprGrammar : proto::or_<
		// VecElmGrammar( index )
		proto::when<
			proto::function< VecElmGrammar< MultithreadingType >, proto::_ >,
			VecElmGrammar< MultithreadingType >(proto::_left, proto::_right)
		>,
		// Vector
		proto::terminal< Vector< MultithreadingType > >,

		// VecExprGrammar +(-) VecExprGrammar
		proto::plus< VecExprGrammar< MultithreadingType >,
					VecExprGrammar< MultithreadingType > > ,
		proto::minus< VecExprGrammar< MultithreadingType >,
					VecExprGrammar< MultithreadingType > >,

		// Vector * double , or double * Vector
		proto::multiplies< proto::terminal< Vector< MultithreadingType > > ,
						proto::terminal< double > >,
		proto::multiplies< proto::terminal< double >,
						proto::terminal< Vector< MultithreadingType > > >,
		// Matrix * Vector
		MatVecMultGrammar< MultithreadingType >,

		// DiagonalMatrix * VecExprGrammar
		proto::multiplies<
			proto::terminal< DiagonalMatrix< MultithreadingType > >,
			VecExprGrammar< MultithreadingType >
		>
	> {};


	template < typename MultithreadingType >
	struct MatDiagmatMatMult;
	// struct LazyMatDiagmatMatMult;


	// The transformation rule for matrix element expressions
	template < typename MultithreadingType >
	struct MatElmGrammar : proto::or_<
		// Matrix
		proto::when< proto::terminal< Matrix< MultithreadingType > >,
					proto::_make_function( proto::_,
										proto::_state, proto::_data) >,

		// Matrix * DiagonalMatrix * Matrix
		proto::when<
			proto::multiplies<
				proto::multiplies<
					proto::terminal< Matrix< MultithreadingType > >,
					proto::terminal< DiagonalMatrix< MultithreadingType > >
				>,
				proto::terminal< Matrix< MultithreadingType > >
			>,
			proto::_make_function(
				MatDiagmatMatMult< MultithreadingType >(
						proto::_value( proto::_left( proto::_left ) ),
						proto::_value( proto::_right( proto::_left) ),
						proto::_value( proto::_right )
				),
				proto::_state, proto::_data
			)
		>,

		// MatElmGrammar +(-) MatElmGrammar
		proto::plus< MatElmGrammar< MultithreadingType >,
					MatElmGrammar< MultithreadingType > > ,
		proto::minus< MatElmGrammar< MultithreadingType >,
					MatElmGrammar< MultithreadingType > >
	> {};


	// The transformation rule for diagonal matrix element expressions
	// with two indices
	template < typename MultithreadingType >
	struct DiagMatElmTwoIdxGrammar : proto::or_<
		// DiagonalMatrix
		proto::when< proto::terminal< DiagonalMatrix< MultithreadingType > >,
					proto::_make_function( proto::_,
											proto::_state, proto::_data) >,

		// DiagMatElmTwoIdxGrammar +(-) DiagMatElmTwoIdxGrammar
		proto::plus< DiagMatElmTwoIdxGrammar< MultithreadingType >,
					DiagMatElmTwoIdxGrammar< MultithreadingType > >,
		proto::minus< DiagMatElmTwoIdxGrammar< MultithreadingType >,
					DiagMatElmTwoIdxGrammar< MultithreadingType > >
	> {};


	// The transformation rule for diagonal matrix element expressions
	// with one index
	template < typename MultithreadingType >
	struct DiagMatElmOneIdxGrammar : proto::or_<
		// DiagonalMatrix
		proto::when< proto::terminal< DiagonalMatrix< MultithreadingType > >,
					proto::_make_function( proto::_, proto::_state) >,

		// DiagMatElmOneIdxGrammar +(-) DiagMatElmOneIdxGrammar
		proto::plus< DiagMatElmOneIdxGrammar< MultithreadingType >,
					DiagMatElmOneIdxGrammar< MultithreadingType > >,
		proto::minus< DiagMatElmOneIdxGrammar< MultithreadingType >,
					DiagMatElmOneIdxGrammar< MultithreadingType > >
	> {};


	// The tranformation rule for diagonal matrix expressions
	template < typename MultithreadingType >
	struct DiagMatExprGrammar : proto::or_<
		// DiagMatElmTwoIdxGrammar( rowIndex, columnIndex )
		proto::when<
			proto::function< DiagMatElmTwoIdxGrammar< MultithreadingType >,
							proto::_, proto::_ >,
			DiagMatElmTwoIdxGrammar< MultithreadingType >(
									proto::_child0,
									proto::_child1, proto::_child2
			)
		>,

		// DiagMatElmOneIdxGrammar( index )
		proto::when<
			proto::function< DiagMatElmOneIdxGrammar< MultithreadingType >,
							proto::_ >,
			DiagMatElmTwoIdxGrammar< MultithreadingType >(
				proto::_child0, proto::_child1
			)
		>,

		// DiagonalMatrix
		proto::terminal< DiagonalMatrix< MultithreadingType > >,

		// DiagMatExprGrammar +(-) DiagMatExprGrammar
		proto::plus< DiagMatExprGrammar< MultithreadingType >,
					DiagMatExprGrammar< MultithreadingType > >,
		proto::minus< DiagMatExprGrammar< MultithreadingType >,
					DiagMatExprGrammar< MultithreadingType > >
	> {};


	// The tranformation rule for matrix expressions
	template < typename MultithreadingType >
	struct MatExprGrammar : proto::or_<
		// MatElmGrammar( rowIndex, columnIndex )
		proto::when<
				proto::function< MatElmGrammar< MultithreadingType >,
								proto::_ , proto::_ >,
				MatElmGrammar< MultithreadingType >(
					proto::_child0, proto::_child1, proto::_child2
				)
			>,

		// Matrix
		proto::terminal< Matrix< MultithreadingType > >,

		// MatExprGrammar * MatExprGrammar
		proto::multiplies< MatExprGrammar< MultithreadingType >,
						MatExprGrammar< MultithreadingType > >,

		// MatExprGrammar * DiagMatExprGrammar
		proto::multiplies< MatExprGrammar< MultithreadingType >,
						DiagMatExprGrammar< MultithreadingType > >,

		// DigaMatExprGrammar * MatExprGrammar
		proto::multiplies< DiagMatExprGrammar< MultithreadingType >,
						MatExprGrammar< MultithreadingType > >,

		// MatExprGrammar +(-) MatExprGrammar
		proto::plus< MatExprGrammar< MultithreadingType >,
					MatExprGrammar< MultithreadingType > > ,
		proto::minus< MatExprGrammar< MultithreadingType >,
					MatExprGrammar< MultithreadingType > >
	> {};


	// The tranformation rule for linear algebraic expressions
	template < typename MultithreadingType >
	struct ExprGrammar : proto::or_<
		VecExprGrammar< MultithreadingType >,
		MatExprGrammar< MultithreadingType >,
		DiagMatExprGrammar< MultithreadingType >
	> {};

	struct ExprGrammarAllParTypes : proto::or_<
		ExprGrammar< PAR::SingleThread >,
		ExprGrammar< PAR::OpenMP >
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
