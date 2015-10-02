/*
 * transformingMatrixVectorMultiplication.cpp
 *
 *  Created on: 2015/09/30
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

namespace LinAlg {
	class Vector;
	class Matrix;

	struct MatVecMultElm;

	/*
	// The transformation rule for vector element expressions
	// This transform accepts a subscript index  of an expression being parsed
	// as the state variable,and distribute that index over the child nodes.
	struct VecElmTrans : proto::or_<
		proto::when< proto::terminal< Vector >,
					proto::_make_function( proto::_, proto::_state) >,
		proto::plus< VecElmTrans, VecElmTrans > ,
		proto::minus< VecElmTrans, VecElmTrans >
	> {};

	// The transformation rule for matrix element expressions
	struct MatElmTrans : proto::or_<
		proto::when< proto::terminal< Matrix>,
					proto::_make_function( proto::_,
										proto::_state, proto::_data) >,
		proto::plus< MatElmTrans, MatElmTrans> ,
		proto::minus< MatElmTrans, MatElmTrans>
	> {};
	*/


	// The transformation rule for element expressions of a vector
	// which results from multiplicaton of matrix and vector
	struct MatVecMultElmTrans : proto::or_<
		proto::when< proto::multiplies< Matrix, Vector>,
					proto::_make_function( MatVecMultElm,
											proto::_left, proto::_right,
											proto::_state) > //,
		// proto::plus< MatVecMultElmTrans, MatVecMultElmTrans>,
		// proto::minus< MatVecMultElmTrans, MatVecMultElmTrans>
	> {};


	// The tranformation rule for vector expressions]
	struct VecExprTrans : proto::or_<
		// proto::when< proto::function< VecElmTrans, proto::_ >,
		//			VecElmTrans(proto::_left, proto::_right) >,
		proto::when< proto::function< MatVecMultElmTrans, proto::_>,
					MatVecMultElmTrans( proto::_left, proto::_right) >,
		// proto::plus< VecExprTrans, VecExprTrans> ,
		// proto::minus< VecExprTrans, VecExprTrans>,
		proto::multiplies< Matrix, Vector>
	> {};

	/*
	// The tranformation rule for matrix expressions
	struct MatExprTrans : proto::or_<
		proto::when<
				proto::function< MatElmTrans, proto::_ , proto::_ >,
				MatElmTrans(proto::_child0, proto::_child1, proto::_child2)
			>,
			proto::terminal< Matrix >,
			proto::plus< MatExprTrans, MatExprTrans> ,
			proto::minus< MatExprTrans, MatExprTrans>
	> {};
	*/

	// The tranformation rule for linear algebraic expressions
	struct ExprTrans : proto::or_<
		VecExprTrans //,
		// MatExprTrans
	> {};


	template<typename E> struct Expr;

	// The above grammar is associated with this domain.
	struct Domain
		: proto::domain<proto::generator<Expr>, ExprTrans>
	{};

	//
	// Linear Algebraic Expression Templates
	//
	template<typename E>
	struct Expr
		: proto::extends<E, Expr<E>, Domain>
	{
		explicit Expr(const E& e)
			: proto::extends<E, Expr<Expr>, Domain>(e)
		{}
	};


	//
	// Vector data are stored in an heap array.
	//
	class Vector {
		private:
			int sz;
			double* data;

	public:
		// template <typename Sig> struct result;

		// template <typename This, typename T>
		// struct result< This(T) > { typedef double type; };

		typedef double result_type;

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
		double& operator()(int i) { return data[i]; }
		const double& operator()(int i) const { return data[i]; }

		// assigning the lhs of a vector expression into this vector
		template<typename Expr>
		Vector& operator=( const Expr& expr ) {
			proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] = trans( VecExprTrans()( expr(i) ) );
			return *this;
		}

		// assigning and adding the lhs of a vector expression into this vector
		template<typename Expr>
		Vector& operator+=( const Expr& expr ) {
			proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] += trans( VecExprTrans()( expr(i) ) );
			return *this;
		}
	};


	//
	// Matrix data are stored in an heap array.
	//
	class Matrix
	{
	private:
		int rowSz, colSz;
		double* data;
		double** m;

	public:
		template <typename Signature> struct result;

		template <typename This, typename T>
		struct result< This(T,T) > { typedef double type; };

		explicit Matrix(int rowSize = 1, int columnSize =1, double iniVal = 0.0) :
			rowSz( rowSize), colSz(columnSize),
			data( new double[rowSz*colSz] ), m( new double*[rowSz])
		{
			for (int i = 0; i < rowSz; i++) m[i] = data + i*colSz;
			for (int ri = 0; ri < rowSz; ri++)
				for (int ci = 0; ci < colSz; ci++) m[ri][ci] = iniVal;
			std::cout << "Created" << std::endl;
		}

		Matrix(const Matrix& mat) :
			rowSz( mat.rowSz), colSz( mat.colSz),
			data( new double[rowSz*colSz] ), m( new double*[rowSz])
		{
				for (int i = 0; i < rowSz; i++) m[i] = data + i*colSz;
						for (int ri = 0; ri < rowSz; ri++)
							for (int ci = 0; ci < colSz; ci++)
								m[ri][ci] = mat.m[ri][ci];
			std::cout << "Copied" << std::endl;
		}

		~Matrix()
		{
			delete [] m;
			delete [] data;
			std::cout << "Deleted" << std::endl;
		}

		int rowSize() const { return rowSz; }
		int columnSize() const { return colSz; }

		// accesing to a vector element
		double& operator()(int ri, int ci) { return m[ri][ci]; }
		const double& operator()(int ri, int ci) const { return m[ri][ci]; }

		/*
		// assigning the lhs of a vector expression into this vector
		template<typename Expr>
		Matrix& operator=( const Expr& expr ) {
			proto::_default<> trans;
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] = trans( MatExprTrans()( expr(ri, ci) ) );
			return *this;
		}

		// assigning and adding the lhs of a vector expression into this vector
		template<typename Expr>
		Matrix& operator+=( const Expr& expr ) {
			proto::_default<> trans;
			for(int ri=0; ri < rowSz; ri++)
				for (int ci=0; ci < colSz; ci++ )
					m[ri][ci] += trans( MatExprTrans()( expr(ri, ci) ) );
			return *this;
		}
		*/
	};

	struct MatVecMultElm
	{
		double operator()( Matrix const& mat, Vector const& vec,
						int index) const
		{
			double elm = 0.0;
			for (int ci =0;  ci < mat.columnSize(); ci++)
				elm += mat(index, ci) * vec(ci);
			return elm;
		}
	};


	// Define a trait for detecting linear algebraic terminals, to be used
	// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
	template<typename> struct IsExpr  : mpl::false_ {};
	template<> struct IsExpr< Vector> : mpl::true_  {};
	template<> struct IsExpr< Matrix> : mpl::true_  {};

	// This defines all the overloads to make expressions involving
	// Vector and Matrix objects to build linear algebraic expression
	// templates.
	// BOOST_PROTO_DEFINE_OPERATORS(IsExpr, proto::default_domain)
	BOOST_PROTO_DEFINE_OPERATORS(IsExpr, Domain)

}

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



int main()
{
	using namespace LinAlg;

	ExpressionSyntaxChecker< ExprTrans >
		syntaxChecker = ExpressionSyntaxChecker< ExprTrans >();
    proto::_default<> trans;

    Matrix a( 3, 3);
    Vector x(3), b(3), r(3);

    a(0,0) = 1.00; a(0,1) = 1.01; a(0,2) = 1.02;
    a(1,0) = 1.10; a(1,1) = 1.11; a(1,2) = 1.12;
    a(2,0) = 1.20; a(2,1) = 1.21; a(2,2) = 1.22;

    x(0) = 1.0;
    x(1) = 2.0;
    x(2) = 3.0;

    b(0) = 0.5;
    b(1) = 0.5;
    b(2) = 0.5;

    std::cout << "Checking if a * x matches to ExprTrans rule ..."
    		<< std::endl;
    syntaxChecker( a * x );
    // syntaxChecker( a + x ); // compile error
    // syntaxChecker( a + a ); // no compile error

    r = a * x;
    // Vector r = b - a * x;
    std::cout << "r =";
    std::cout << '(' << r(0) << ',' << r(1)
              << ',' << r(2) << ')' << std::endl;

	return 0;
}


