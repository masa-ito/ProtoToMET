/*
 * matVecMultAndVecSubMetaOpenMP.hpp
 *
 *  Created on: 2016/05/16
 *      Author: Masakatsu ITO
 */

#ifndef MATVECMULTANDVECSUBMETAOPENMP_HPP_
#define MATVECMULTANDVECSUBMETAOPENMP_HPP_

#include <iostream>
#include <boost/proto/proto.hpp>

#include <ParallelizationTypeTag/Default.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

namespace PTT = ParallelizationTypeTag;

namespace DenseLinAlg {

	class Vector;
	class Matrix;


	// Callable transform object to make a proto exression
	// for lazily evaluating multiplication
	struct MatVecMult;
	struct MatVecMultOmp;

	// The grammar for the multiplication of a matrix and a vector
	struct MatVecMultGrammar : proto::or_<
		proto::when<
			proto::multiplies< proto::terminal< Matrix > ,
								proto::terminal< Vector> >,
			MatVecMult( proto::_value( proto::_left),
						proto::_value( proto::_right) )
		>
	> {};

	// The grammar for the multiplication of a matrix and a vector
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
	struct VecElementwiseElmGrammar : proto::or_<
		// Vector
		proto::when< proto::terminal< Vector >,
					proto::_make_function( proto::_, proto::_state) >,

		// double * Vector

		// DiagonalMatrix * Vector

		// VecElementwiseElmGrammar +(-) VecElementwiseElmGrammar
		proto::plus< VecElementwiseElmGrammar, VecElementwiseElmGrammar > ,
		proto::minus< VecElementwiseElmGrammar, VecElementwiseElmGrammar >
	> {};

	// The transformation rule for vector element expressions
	// This transform accepts a subscript index  of an expression being parsed
	// as the state variable,and distribute that index over the child nodes.
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

	// The transformation rule for vector element expressions
	// This transform accepts a subscript index  of an expression being parsed
	// as the state variable,and distribute that index over the child nodes.
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
		proto::minus< VecElementwiseGrammar, VecElementwiseGrammar > //,

		// Vector * double , or double * Vector

		// DiagonalMatrix * VecExprGrammar
	> {};

	// The grammar for a vector expression
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

	// The grammar for a vector expression
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

	// The transformation rule for matrix element expressions
	struct MatElmGrammar : proto::or_<
		// Matrix
		proto::when<
			proto::terminal< Matrix >,
			proto::_make_function( proto::_,
										proto::_state, proto::_data)
		> // ,

		// Matrix * DiagonalMatrix * Matrix

		// MatElmGrammar +(-) MatElmGrammar
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
		proto::terminal< Matrix > //,

		// MatExprGrammar * MatExprGrammar
		// MatExprGrammar * DiagMatExprGrammar
		// DigaMatExprGrammar * MatExprGrammar
		// MatExprGrammar +(-) MatExprGrammar
	> {};

	// The tranformation rule for linear algebraic expressions
	struct ExprGrammar : proto::or_<
		VecExprGrammar,
		MatExprGrammar // ,
		// DiagMatExprGrammar
	> {};

	// Callable transform object
	// for lazily assigning a vector expression into a vector
	struct AssignVecExpr;
	// struct PlusAssignVecExpr;

	// The transformation rule for assigning a vector expression
	// into a vector object
	struct AssignVecExprGrammar : proto::when<
		VecExprGrammar,
		AssignVecExpr( proto::_, proto::_state, proto::_data)
	> {};

	/* struct PlusAssignVecExprTrans : proto::when<
		VecExprTrans,
		PlusAssignVecExpr( proto::_, proto::_state, proto::_data)
	> {}; */


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


	// A wrapper for a linear algebraic expression
	template<typename E> struct ExprWrapper;

	// The above grammar is associated with this domain.
	struct Domain
		: proto::domain<proto::generator<ExprWrapper>, ExprGrammar>
	{};

	// A wrapper template for linear algebraic expressions
	// including matrices and vectors
	template<typename ExprType>
	struct ExprWrapper
		: proto::extends<ExprType, ExprWrapper<ExprType>, Domain>
	{
		explicit ExprWrapper(const ExprType& e)
			: proto::extends<ExprType, ExprWrapper<ExprType>, Domain>(e) /*,
			  rowSz( e.rowSize()), colSz( e.columnSize()) */
		{}

	};


	class Vector {
	private:
		const int sz;
		double* data;

	public:
		template <typename Sig> struct result;

		template <typename This, typename T>
		struct result< This(T) > { typedef double type; };

		explicit Vector(int sz_, double iniVal) :
			sz( sz_), data( new double[sz] ) {
			for (int i = 0; i < sz; i++) data[i] = iniVal;
			// std::cout << "Created" << std::endl;
		}

		// No initialization
		explicit Vector(int sz_ = 1) :
			sz( sz_), data( new double[sz] ) {
			// std::cout << "Created" << std::endl;
		}

		/* Vector(const Vector& vec) :
			sz( vec.sz), data( new double[sz] ) {
			for (int i = 0; i < sz; i++) data[i] = vec.data[i];
			// std::cout << "Copied! " << std::endl;
		} */

		/* template < typename Derived >
		Vector( const LazyVectorMaker< Derived > & maker) :
			sz( maker.columnSize()), data( new double[sz] )
		{
			maker.assignDataTo( *this);
		} */

		~Vector() {
			delete [] data;
			// std::cout << "Deleted" << std::endl;
		}

		int size() const { return sz; }
		int rowSize() const { return 1; }
		int columnSize() const { return sz; }

		/* double dot( const Vector& vec) const
		{
			double d = data[0] * vec.data[0];
			for (int i = 1; i < sz; i++) d += data[i] * vec.data[i];
			return d;
		} */

		/* double abs() const {
			double aSqr = data[0] * data[0];
			for (int i = 1; i < sz; i++) aSqr += data[i] * data[i];
			return sqrt( aSqr);
		} */

		// accessing to an element of this vector
		double& operator()(int i) { return data[i]; }
		const double& operator()(int i) const { return data[i]; }

		// assigning the lhs of a vector expression into this vector
		template<typename Expr>
		Vector& operator=( const Expr& expr ) {
			AssignVecExprGrammar()( expr, *this, PTT::Specified());
			return *this;
		}

		/* template < typename Derived >
		Vector& operator=( const LazyVectorMaker< Derived > & maker) {
			maker.assignDataTo( *this);
			return *this;
		} */


		// assigning and adding the lhs of a vector expression into this vector
		/* template<typename Expr>
		Vector& operator+=( const Expr& expr ) {
			for(int i=0; i < sz; ++i)
				data[i] += VecExprGrammar()( expr(i) );
			return *this;
		} */

		// assigning and subtracting the lhs of a vector expression into
		// this vector
		/* template<typename Expr>
		Vector& operator-=( const Expr& expr ) {
			for(int i=0; i < sz; ++i)
				data[i] -= VecExprGrammar()( expr(i) );
			return *this;
		} */

		/* friend void diagPrecondConGrad( Vector & ansVec,
				const Matrix & coeffMat, const Vector & rhsVec,
				const Vector & initGuessVec,
				double convergenceCriterion); */

		friend class AssignVecExpr;
		// friend class PlusAssignVecExpr;
	};


	class Matrix
	{
	private:
		int rowSz, colSz;
		double* data;
		double** m;

	public:
		template <typename Sig> struct result;

		template <typename This, typename T>
		struct result< This(T,T) > { typedef double type; };

		explicit Matrix(int rowSize = 1, int columnSize =1,
					double iniVal = 0.0) :
			rowSz( rowSize), colSz(columnSize),
			data( new double[rowSz*colSz] ), m( new double*[rowSz])
		{
			for (int i = 0; i < rowSz; i++) m[i] = data + i*colSz;
			for (int ri = 0; ri < rowSz; ri++)
				for (int ci = 0; ci < colSz; ci++) m[ri][ci] = iniVal;
		}

		/* Matrix( const Matrix& mat) :
			rowSz( mat.rowSz), colSz( mat.colSz),
			data( new double[rowSz*colSz] ), m( new double*[rowSz])
		{
			for (int i = 0; i < rowSz; i++) m[i] = data + i*colSz;
				for (int ri = 0; ri < rowSz; ri++)
					for (int ci = 0; ci < colSz; ci++)
						m[ri][ci] = mat.m[ri][ci];
		} */

		~Matrix()
		{
			delete [] m;
			delete [] data;
		}

		int rowSize() const { return rowSz; }
		int columnSize() const { return colSz; }

		// accesing to a matrix element
		double& operator()(int ri, int ci) { return m[ri][ci]; }
		const double& operator()(int ri, int ci) const { return m[ri][ci]; }

		// assigning the lhs of a vector expression into this matrix
		/* template<typename Expr>
		Matrix& operator=( const ExprWrapper< Expr >& expr ) {
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] = MatExprGrammar()( expr(ri, ci) );
			return *this;
		} */

		/* template < typename Derived >
		Matrix& operator=( const LazyMatrixMaker< Derived >& maker) {
			maker.assignDataTo( *this);
			return *this;
		} */

		// assigning and adding the lhs of a vector expression into this matrix
		/* template<typename Expr>
		Matrix& operator+=( const Expr& expr ) {
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] += MatExprGrammar()( expr(ri, ci) );
			return *this;
		}*/

		// assigning and subtracting the lhs of a vector expression into
		// this matrix
		/* template<typename Expr>
		Matrix& operator-=( const Expr& expr ) {
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] -= MatExprGrammar()( expr(ri, ci) );
			return *this;
		} */

		/* friend void diagPrecondConGrad( Vector & ansVec,
				const Matrix & coeffMat, const Vector & rhsVec,
				const Vector & initGuessVec,
				double convergenceCriterion); */
	};


	// Primitive transform object for lazily implemeting
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
					rhs.data[i] = VecExprGrammar()( expr(i) );
			return;
		}

		template < typename Expr >
		typename boost::enable_if<
			proto::matches< Expr, VecElementwiseGrammar >, result_type
		>::type
		operator()( const Expr& expr, Vector& rhs,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) const
		{
			#pragma omp parallel for
			for(int i=0; i < rhs.sz; ++i)
					rhs.data[i] = VecElementwiseGrammar()( expr(i) );
			return;
		}

		template < typename Expr >
		typename boost::enable_if<
			proto::matches< Expr, VecReductionGrammar >, result_type
		>::type
		operator()( const Expr& expr, Vector& rhs,
				const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& ) const
		{
			for(int i=0; i < rhs.sz; ++i)
					rhs.data[i] = VecReductionOmpGrammar()( expr(i) );
			return;
		}
	};


	// Lazy function object for evaluating an element of
	// the resultant vector from the multiplication of
	// matrix and vector objects.
	//
	// An expression like ( matrix * vector )(index) is transformed
	// into the loop for calculating the dot product between
	// the index'th row of the matrix and the vector.
	struct LazyMatVecMult
	{
		Matrix const& m;
		Vector const& v;
		const int mColSz;

		typedef double result_type;

		explicit LazyMatVecMult(Matrix const& mat, Vector const& vec) :
			m( mat), v( vec), mColSz(mat.rowSize()) {}

		LazyMatVecMult( LazyMatVecMult const& lazy) :
			m(lazy.m), v(lazy.v), mColSz(lazy.mColSz) {}

		result_type operator()(int index) const
		{
			result_type elm = 0.0;
			for (int ci =0;  ci < mColSz; ci++)
				elm += m(index, ci) * v(ci);
			return elm;
		}
	};

	struct LazyMatVecMultOmp
	{
		Matrix const& m;
		Vector const& v;
		const int mColSz;

		typedef double result_type;

		explicit LazyMatVecMultOmp(Matrix const& mat, Vector const& vec) :
			m( mat), v( vec), mColSz(mat.rowSize()) {}

		LazyMatVecMultOmp( LazyMatVecMultOmp const& lazy) :
			m(lazy.m), v(lazy.v), mColSz(lazy.mColSz) {}

		result_type operator()(int index) const
		{
			result_type elm = 0.0;

			#pragma omp parallel for reduction (+:elm)
			for (int ci =0;  ci < mColSz; ci++)
				elm += m(index, ci) * v(ci);
			return elm;
		}
	};

	// Callable transform object to make the lazy functor
	// a proto exression for lazily evaluationg the multiplication
	// of a matrix and a vector .
	struct MatVecMult : proto::callable
	{
		typedef proto::terminal< LazyMatVecMult >::type result_type;

		result_type
		operator()( Matrix const& mat, Vector const& vec) const
		{
			return proto::as_expr( LazyMatVecMult(mat, vec) );
		}
	};

	// Callable transform object to make the lazy functor
	// a proto exression for lazily evaluationg the multiplication
	// of a matrix and a vector .
	struct MatVecMultOmp : proto::callable
	{
		typedef proto::terminal< LazyMatVecMultOmp >::type result_type;

		result_type
		operator()( Matrix const& mat, Vector const& vec) const
		{
			return proto::as_expr( LazyMatVecMultOmp(mat, vec) );
		}
	};

	// Define a trait for detecting linear algebraic terminals, to be used
	// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
	template<typename> struct IsExpr  : mpl::false_ {};

	template<> struct IsExpr< Vector > : mpl::true_  {};
	template<> struct IsExpr< Matrix > : mpl::true_  {};
	// template<> struct IsExpr< DiagonalMatrix > : mpl::true_  {};

	template<> struct IsExpr< LazyMatVecMult > : mpl::true_  {};

	// template<> struct IsExpr< LazyMatDiagmatMatMult > : mpl::true_  {};

	// This defines all the overloads to make expressions involving
	// Vector and Matrix objects to build Proto's expression templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsExpr, Domain)

}



#endif /* MATVECMULTANDVECSUBMETAOPENMP_HPP_ */
