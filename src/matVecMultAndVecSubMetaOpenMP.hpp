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

	// The grammar for the multiplication of a matrix and a vector
	struct MatVecMultGrammar : proto::or_<
		proto::when<
			proto::multiplies< proto::terminal< Matrix > ,
								proto::terminal< Vector> >,
			MatVecMult( proto::_value( proto::_left),
						proto::_value( proto::_right) )
		>
	> {};

	// The transformation rule for vector element expressions
	// This transform accepts a subscript index  of an expression being parsed
	// as the state variable,and distribute that index over the child nodes.
	struct VecMapElmGrammar : proto::or_<
		// Vector
		proto::when< proto::terminal< Vector >,
					proto::_make_function( proto::_, proto::_state) >,

		// double * Vector

		// DiagonalMatrix * Vector

		// VecMapElmGrammar +(-) VecMapElmGrammar
		proto::plus< VecMapElmGrammar, VecMapElmGrammar > ,
		proto::minus< VecMapElmGrammar, VecMapElmGrammar >
	> {};

	// The transformation rule for vector element expressions
	// This transform accepts a subscript index  of an expression being parsed
	// as the state variable,and distribute that index over the child nodes.
	struct VecMapReduceElmGrammar : proto::or_<
		// Matrix * Vector
		proto::when<
			MatVecMultGrammar,
			proto::_make_function( MatVecMultGrammar( proto::_),
									proto::_state )
		>,

		// VecMapReduceElmGrammar +(-) VecMapReduceElmGrammar
		proto::plus< VecMapReduceElmGrammar, VecMapReduceElmGrammar > ,
		proto::minus< VecMapReduceElmGrammar, VecMapReduceElmGrammar > ,

		// VecMapReduceElmGrammar +(-) VecMapElmGrammar
		proto::plus< VecMapReduceElmGrammar, VecMapElmGrammar > ,
		proto::minus< VecMapReduceElmGrammar, VecMapElmGrammar > ,

		// VecMapElmGrammar +(-) VecMapReduceElmGrammar
		proto::plus< VecMapElmGrammar, VecMapReduceElmGrammar > ,
		proto::minus< VecMapElmGrammar, VecMapReduceElmGrammar >
	> {};


	// The grammar for a vector expression
	struct VecMapGrammar : proto::or_<
		// VecMapElmGrammar( index )
		proto::when<
			proto::function< VecMapElmGrammar, proto::_ >,
			// VecMapElmGrammar( proto::_left, proto::_right)
			proto::_default< >(
					VecMapElmGrammar(proto::_left, proto::_right) )
		>,

		// Vector
		proto::terminal< Vector >,

		// VecMapGrammar +(-) VecMapGrammar
		proto::plus< VecMapGrammar, VecMapGrammar > ,
		proto::minus< VecMapGrammar, VecMapGrammar > //,

		// Vector * double , or double * Vector

		// DiagonalMatrix * VecExprGrammar
	> {};

	// The grammar for a vector expression
	struct VecMapReduceGrammar : proto::or_<
		// VecMapReduceElmGrammar( index )
		proto::when<
			proto::function< VecMapReduceElmGrammar, proto::_ >,
			proto::_default< >(
					VecMapReduceElmGrammar(proto::_left, proto::_right) )
		>,

		// VecMapReduceGrammar +(-) VecMapReduceGrammar
		proto::plus< VecMapReduceGrammar, VecMapReduceGrammar > ,
		proto::minus< VecMapReduceGrammar, VecMapReduceGrammar >,

		// VecMapReduceGrammar +(-) VecMapGrammar
		proto::plus< VecMapReduceGrammar, VecMapGrammar > ,
		proto::minus< VecMapReduceGrammar, VecMapGrammar >,

		// VecMapGrammar +(-) VecMapReduceGrammar
		proto::plus< VecMapGrammar, VecMapReduceGrammar > ,
		proto::minus< VecMapGrammar, VecMapReduceGrammar >,

		// Matrix * Vector
		proto::multiplies< proto::terminal< Matrix > ,
							proto::terminal< Vector> > //,
	> {};

	// The grammar for a vector expression
	struct VecExprGrammar : proto::or_<
		VecMapGrammar,
		VecMapReduceGrammar
		/* proto::when<
			VecMapGrammar,
			proto::_default< >( VecMapGrammar( proto::_ ) )
		>,
		proto::when<
			VecMapReduceGrammar,
			proto::_default< >( VecMapReduceGrammar( proto::_ ) )
		> */
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


	//
	// Tag hierarchy of vector expression type tags
	//
	struct VecExprTag {};
	struct VecMapTag : VecExprTag {};
	struct VecMapReduceTag : VecMapTag {};

	// Meta function returning an instance of Vector expression type tag
	struct VecExprTagGrammar : proto::or_<
		proto::when<
			VecMapReduceGrammar,
			// proto::_make_function( VecMapReduceTag)
			VecMapReduceTag()
		>,

		proto::when<
			VecMapGrammar,
			// proto::_make_function( VecMapTag)
			VecMapTag()
		>,

		proto::when<
			VecExprGrammar,
			// proto::_make_function( VecExprTag)
			VecExprTag()
		>
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

	// These classes are to be used as the template parameter
	// for AssignType.
	struct AssignFunctor {
		void operator()( double& lhs, double rhs) const { lhs = rhs; }
	};

	template < typename AssignType > struct AssignVecExpr;


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
			AssignVecExpr< AssignFunctor >()(
					expr, VecExprTagGrammar()( expr),
					*this, PTT::Specified()
			);
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

		template < typename AssignType > friend class AssignVecExpr;
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


	// Primitive transform object for lazily assigning
	// an vector expresion into a vector object
	template < typename AssignType >
	struct AssignVecExpr
	{
		template < typename Expr >
		void
		operator()( const Expr& expr, const VecExprTag&, Vector& lhs,
			const PTT::SingleProcess< PTT::SingleThread< PTT::NoSIMD > >& )
		const
		{
			for(int i=0; i < lhs.sz; ++i)
				AssignType()( lhs.data[i], VecExprGrammar()( expr(i) ) );
		}

		template < typename Expr >
		void
		operator()( const Expr& expr, const VecMapTag&, Vector& lhs,
			const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& )
		const
		{
			#pragma omp parallel for
			for(int i=0; i < lhs.sz; ++i)
				AssignType()( lhs.data[i], VecMapGrammar()( expr(i) ) );
			std::cout << "skelton for Map, OpenMP " << std::endl;
		}

		template < typename Expr >
		void
		operator()( const Expr& expr, const VecMapReduceTag&, Vector& lhs,
			const PTT::SingleProcess< PTT::OpenMP< PTT::NoSIMD > >& )
		const
		{
			#pragma omp parallel for shared( lhs)
			for(int i=0; i < lhs.sz; ++i)
				AssignType()( lhs.data[i], VecMapReduceGrammar()( expr(i) ) );
			std::cout << "skelton for Map and Reduce, OpenMP " << std::endl;
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

	// Callable transform object to make the lazy functor
	// a proto exression for lazily evaluationg the multiplication
	// of a matrix and a vector .
	struct MatVecMult : proto::callable
	{
		typedef proto::terminal< LazyMatVecMult >::type result_type;

		result_type
		operator()( const Matrix& mat, const Vector& vec) const
		{
			return proto::as_expr( LazyMatVecMult(mat, vec) );
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
