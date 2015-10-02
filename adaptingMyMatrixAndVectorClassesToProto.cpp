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

	// Functor for lazy evaluation
	struct ElmOfMatVecMult;

	// The grammar for an expression like ( matrix * vector )(index)
	//
	// ( Note that ( matrix * vector ) gives another vector, so that
	// its index'th element is given by ( matrix * vector )(index) . )
	struct MatVecMultElmGrammar : proto::or_<
		proto::when< proto::multiplies< Matrix, Vector>,
					proto::_make_function( ElmOfMatVecMult,
											proto::_left, proto::_right,
											proto::_state) >
	> {};

	// The grammar for a vector expression
	// ( Now I consider just one form : matrix * vector . )
	struct VecExprGrammar : proto::or_<
		proto::when< proto::function< MatVecMultElmGrammar, proto::_>,
					MatVecMultElmGrammar( proto::_left, proto::_right) >,
		proto::multiplies< Matrix, Vector>
	> {};


	template<typename E> struct Expr;

	// The above grammar is associated with this domain.
	struct Domain
		: proto::domain<proto::generator<Expr>, VecExprGrammar>
	{};

	// A wrapper template for linear algebraic expressions
	// including matrices and vectors
	template<typename E>
	struct Expr
		: proto::extends<E, Expr<E>, Domain>
	{
		explicit Expr(const E& e)
			: proto::extends<E, Expr<Expr>, Domain>(e)
		{}

		typedef double result_type;
	};


	// Vector data are stored in an heap array.
	class Vector {
		private:
			int sz;
			double* data;

	public:
		template <typename Sig> struct result;

		template <typename This, typename T>
		struct result< This(T) > { typedef double type; };

		typedef double result_type;

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
				data[i] = trans( VecExprGrammar( expr(i) ) );
			return *this;
		}
	};


	// Matrix data are stored in an heap array.
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


	// Functor for lazily evaluating an element of the resultant vector
	// from the multiplication of a matrix and vector
	//
	// An expression like ( matrix * vector )(index) is transformed
	// into the loop for calculating the dot product between
	// the vector and matrix.
	struct ElmOfMatVecMult
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
	// Vector and Matrix objects to build Proto's expression templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsExpr, Domain)
	// BOOST_PROTO_DEFINE_OPERATORS(IsExpr, proto::default_domain)


}


int main()
{
	using namespace LinAlg;

    Matrix mat( 3, 3);
    Vector vec1(3), vec2(3);

    // Let's see if any temporary oject is copied !
    vec2 = mat * vec1;
	return 0;
}





