/*
 * adaptingMyMatrixAndVectorClassesToProto.cpp
 *
 *  Created on: 2015/10/01
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

namespace LinAlg {
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

	// The grammar for a vector expression
	// ( Now I consider just one form : matrix * vector . )
	struct VecExprGrammar : proto::or_<
		proto::function< MatVecMultGrammar, proto::_>,
		MatVecMultGrammar
	> {};


	// A wrapper for a linear algebraic expression
	template<typename E> struct Expr;

	// The above grammar is associated with this domain.
	struct Domain
		: proto::domain<proto::generator<Expr>, VecExprGrammar>
	{};

	// A wrapper template for linear algebraic expressions
	// including matrices and vectors
	template<typename ExprType>
	struct Expr
		: proto::extends<ExprType, Expr<ExprType>, Domain>
	{
		/* typedef double result_type; */

		explicit Expr(const ExprType& e)
			: proto::extends<ExprType, Expr<ExprType>, Domain>(e)
		{}
	};


	// Testing if data in an heap array can be a vector object
	class Vector {
		private:
			int sz;
			double* data;

	public:
		explicit Vector(int sz_ = 1, double iniVal = 0.0) :
			sz( sz_), data( new double[sz] ) {
			for (int i = 0; i < sz; i++) data[i] = iniVal;
			std::cout << "Created" << std::endl;
		}
		Vector(const Vector& vec) :
			sz( vec.sz), data( new double[sz] ) {
			for (int i = 0; i < sz; i++) data[i] = vec.data[i];
			std::cout << "Copied! " << std::endl;
		}

		~Vector() {
			delete [] data;
			std::cout << "Deleted" << std::endl;
		}

		// accessing to an element of this vector
		double& operator()(int i) { return data[i]; }
		const double& operator()(int i) const { return data[i]; }

		// assigning the lhs of a vector expression into this vector
		template<typename Expr>
		Vector& operator=( const Expr& expr ) {
			proto::_default<> trans;
			for(int i=0; i < sz; ++i)
				data[i] = trans( VecExprGrammar()( expr(i) ) );
			return *this;
		}
	};


	// Testing if data in an heap array can be a matrix object
	class Matrix
	{
	private:
		int rowSz, colSz;
		double* data;
		double** m;

	public:

		explicit Matrix(int rowSize = 1, int columnSize =1,
					double iniVal = 0.0) :
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
				std::cout << "Copied! " << std::endl;
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

	};

	// Lazy function object for evaluating an element of
	// the resultant vector from the multiplication of
	// a matrix and vector objects.
	//
	// An expression like ( matrix * vector )(index) is transformed
	// into the loop for calculating the dot product between
	// the index'th row of the matrix and the vector.
	struct LazyMatVecMult
	{
		Matrix const& m;
		Vector const& v;
		int mColSz;

		typedef double result_type;
		// typedef mpl::int_<1> proto_arity;

		explicit LazyMatVecMult(Matrix const& mat, Vector const& vec) :
		m( mat), v( vec), mColSz(mat.rowSize()) {}

		LazyMatVecMult(LazyMatVecMult const& lazy) :
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
		operator()( Matrix const& mat, Vector const& vec) const
		{
			return proto::as_expr( LazyMatVecMult(mat, vec) );
		}
	};

	// Define a trait for detecting linear algebraic terminals, to be used
	// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
	template<typename> struct IsExpr  : mpl::false_ {};
	template<> struct IsExpr< Vector> : mpl::true_  {};
	template<> struct IsExpr< Matrix> : mpl::true_  {};
	// template<> struct IsExpr< MatVecMult> : mpl::true_  {};
	template<> struct IsExpr< LazyMatVecMult> : mpl::true_  {};

	// This defines all the overloads to make expressions involving
	// Vector and Matrix objects to build Proto's expression templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsExpr, Domain)
	// BOOST_PROTO_DEFINE_OPERATORS(IsExpr, proto::default_domain)


}



int main()
{
	using namespace LinAlg;

    std::cout << "Let's see if any temporary oject is copied !" << std::endl;

    Matrix mat( 3, 3);
    Vector vec1(3), vec2(3);

    mat(0,0) = 1.00; mat(0,1) = 1.01; mat(0,2) = 1.02;
    mat(1,0) = 1.10; mat(1,1) = 1.11; mat(1,2) = 1.12;
    mat(2,0) = 1.20; mat(2,1) = 1.21; mat(2,2) = 1.22;

    vec1(0) = 1.0;
    vec1(1) = 2.0;
    vec1(2) = 3.0;

    std::cout << " mat * vec1" << std::endl;
    proto::display_expr( mat * vec1 );
    std::cout << " MatVecMultGrammar()( mat * vec1) " << std::endl;
    proto::display_expr( MatVecMultGrammar()( mat * vec1) );

    std::cout << "( mat * vec1)(2)" << std::endl;
    proto::display_expr( ( mat * vec1)(2) );
    std::cout << "VecExprGrammar()( ( mat * vec1)(2) )" << std::endl;
    proto::display_expr( VecExprGrammar()( ( mat * vec1)(2) ) );

	proto::_default<> trans;
    double elm2 = trans( VecExprGrammar()( ( mat * vec1)(2) ) );

    std::cout << "( mat * vec1)(2) = " << elm2 << std::endl;
    // This should be 7.28 .

    vec2 = mat * vec1;

    std::cout << " vec2 = mat * vec1 = " << std::endl;
    std::cout << " ( " << vec2(0) << ", " << vec2(1) << ", " <<
    		vec2(2) << ")" << std::endl;
    // This should be ( 6.08 , 6.68 , 7.28) .

	return 0;
}





