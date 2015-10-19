/*
 * matVecMul.cpp
 *
 *  Created on: 2015/06/01
 *      Author: Masakatsu ITO
 */


#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

class Vector;
class Matrix;

// This grammar describes which matrix expressions
// are allowed; namely, matrix terminals,
// addition and subtraction of matrix expressions,
// multiplication of matrix expressions.
struct MatGrammar : proto::or_<
	proto::terminal< Matrix >,
	proto::plus< MatGrammar, MatGrammar>,
	proto::minus< MatGrammar, MatGrammar>,
	proto::multiplies< MatGrammar, MatGrammar >
> {};

// This grammar describes which vector expressions
// are allowed; namely, vector terminals,
// addition and subtraction of matrix expressions,
// multiplication of matrix and vector expressions.
struct VecGrammar : proto::or_<
	proto::terminal< Vector >,
	proto::plus< VecGrammar, VecGrammar>,
	proto::minus< VecGrammar, VecGrammar>,
	proto::multiplies< MatGrammar, VecGrammar >
> {};

// This grammar describes which linear algebraic expressions
// are allowed;
struct LinAlgGrammar : proto::or_<
	MatGrammar,
	VecGrammar
> {};

// The above grammar is associated with this domain.
template<typename Expr> struct LinAlgExpr;
struct LinAlgDomain
	: proto::domain<proto::generator<LinAlgExpr>, LinAlgGrammar> {};


//
// Context for evaluating an element of matrix expressions
//
struct MatIndexCntxt
	: proto::callable_context<const MatIndexCntxt>
{
	typedef double result_type;

	int i_, j_;
	MatIndexCntxt(int i, int j) :  i_(i), j_(j) {}

	// matrix element
	template<typename Matrix>
	double operator()(proto::tag::terminal, const Matrix& mat) const
	{
		return mat(i_,j_);
	}

	// addition of matrix expression terms
	template<typename E1, typename E2>
	double operator()(proto::tag::plus, const E1& e1, const E2& e2) const
	{
		return proto::eval(e1, *this) + proto::eval(e2, *this);
	}
	// subtraction of matrix expression terms
	template<typename E1, typename E2>
	double operator()(proto::tag::minus, const E1& e1, const E2& e2) const
	{
		return proto::eval(e1, *this) - proto::eval(e2, *this);
	}
};


//
// Context for evaluating an element of vector expressions
//
struct VecIndexCntxt
	: proto::callable_context<const VecIndexCntxt> {
		typedef double result_type;

		int index;
		VecIndexCntxt(int index_) :  index(index_) {}

		// vector element
		template<typename Vector>
		double operator()(proto::tag::terminal, const Vector& vec) const {
			return vec(index);
		}

		// addition of vector expression terms
		template<typename E1, typename E2>
		double operator()(proto::tag::plus, const E1& e1, const E2& e2) const {
			return proto::eval(e1, *this) + proto::eval(e2, *this);
		}
		// substraction of vector expression terms
		template<typename E1, typename E2>
		double operator()(proto::tag::minus, const E1& e1, const E2& e2) const {
			return proto::eval(e1, *this) - proto::eval(e2, *this);
		}
};

//
// Linear Algebraic Expression Templates
//
template<typename Expr>
struct LinAlgExpr
	: proto::extends<Expr, LinAlgExpr<Expr>, LinAlgDomain> {
		explicit LinAlgExpr(const Expr& e)
			: proto::extends<Expr, LinAlgExpr<Expr>, LinAlgDomain>(e) {
		}

		// Use a MatIndexCntxt instance to implement indexing
		// of a matrix expression tree.
		typename proto::result_of::eval< Expr, MatIndexCntxt>::type
		operator ()(int i, int j) const {
			const MatIndexCntxt mCtx( i, j);
			return proto::eval(*this, mCtx);
		}

		// Use a VecIndexCntxt instance to implement indexing
		// of a vector expression tree.
		typename proto::result_of::eval< Expr, VecIndexCntxt>::type
		operator ()(int i) const {
			const VecIndexCntxt vCtx(i);
			return proto::eval(*this, vCtx);
		}
};

//
// Matrix data are stored in an heap array.
//
class Matrix
{
private:
	int rowSize, colSize;
	double* data;
	double** m;

public:
	explicit Matrix(int rowSize_ = 1, int colSize_ =1, double iniVal = 0.0) :
		rowSize( rowSize_), colSize(colSize_),
		data( new double[rowSize*colSize] ),
		m( new double*[rowSize]) {
		for (int i = 0; i < rowSize; i++) m[i] = data + i*colSize;
		for (int ri = 0; ri < rowSize; ri++)
			for (int ci = 0; ci < colSize; ci++) m[ri][ci] = iniVal;
		std::cout << "Created" << std::endl;
	}
	Matrix(const Matrix& mat) :
		rowSize( mat.rowSize), colSize( mat.colSize),
		data( new double[rowSize*colSize] ),
		m( new double*[rowSize]){
		for (int ri = 0; ri < rowSize; ri++) m[ri] = data + ri*colSize;
		for (int ri = 0; ri < rowSize; ri++)
			for (int ci = 0; ci < colSize; ci++) m[ri][ci] = mat.m[ri][ci];
		std::cout << "Copied" << std::endl;
	}

	~Matrix() {
		delete [] m;
		delete [] data;
		std::cout << "Deleted" << std::endl;
	}

	// accesing to a matrix element
	double& operator()(int i, int j) { return m[i][j]; }
	const double& operator()(int i, int j) const { return m[i][j]; }

	// assigning the lhs of a matrix expression into this matrix
	template<typename Expr>
	Matrix& operator=( const Expr& expr )
	{
		for(int i=0; i< rowSize; ++i)
			for(int j=0; j < colSize; ++j) {
				// evaluating the (i,j) element of a matrix expression
				const MatIndexCntxt mCtx(i, j);
				m[i][j] = proto::eval(proto::as_expr<LinAlgDomain>(expr), mCtx);
			}
		return *this;
	}

	// assigning and adding the lhs of a matrix expression into this matrix
	template<typename Expr>
	Matrix& operator+=( const Expr& expr )
	{
		for(int i=0; i< rowSize; ++i)
			for(int j=0; j < colSize; ++j) {
				// evaluating the (i,j) element of a matrix expression
				const MatIndexCntxt mCtx(i, j);
				m[i][j] += proto::eval(proto::as_expr<LinAlgDomain>(expr), mCtx);
			}
		return *this;
	}
};


//
// Vector data are stored in an heap array.
//
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
		for(int i=0; i < sz; ++i) {
				// evaluating the i'th element of a vector expression
				const VecIndexCntxt vCtx(i);
				data[i] = proto::eval(proto::as_expr<LinAlgDomain>(expr),vCtx);
		}
		return *this;
	}

	// assigning and adding the lhs of a vector expression into this vector
	template<typename Expr>
	Vector& operator+=( const Expr& expr ) {
		for(int i=0; i < sz; ++i) {
				// evaluating the (i,j) element of a matrix expression
				const VecIndexCntxt vCtx(i);
				data[i] += proto::eval(proto::as_expr<LinAlgDomain>(expr), vCtx);
		}
		return *this;
	}
};

// Define a trait for detecting matrix and vector terminals, to be used
// by the BOOST_PROTO_DEFINE_OPERATORS macro below.
template<typename> struct IsLinAlg : mpl::false_ {};
template<> struct IsLinAlg<Matrix> : mpl::true_  {};
template<> struct IsLinAlg<Vector> : mpl::true_  {};



namespace LinAlgOps {
	// This defines all the overloads to make expressions involving
	// Vector objects to build expression templates.
	BOOST_PROTO_DEFINE_OPERATORS(IsLinAlg, LinAlgDomain)
}


int main() {
	using namespace LinAlgOps;

	// lazy_matrices with 2x2 elements each
	Matrix m1( 2, 2, 0.0), m2( 2, 2, 0.0), m3( 2, 2, 0.0);
	m1(0,0) = 1.00;
	m1(0,1) = 1.01;
	m1(1,0) = 1.10;
	m1(1,1) = 1.11;
	std::cout << m1(0,0) << std::endl;
	std::cout << m1(0,1) << std::endl;
	std::cout << m1(1,0) << std::endl;
	std::cout << m1(1,1) << std::endl;

	m2(0,0) = 2.00;
	m2(0,1) = 2.01;
	m2(1,0) = 2.10;
	m2(1,1) = 2.11;
	std::cout << m2(0,0) << std::endl;
	std::cout << m2(0,1) << std::endl;
	std::cout << m2(1,0) << std::endl;
	std::cout << m2(1,1) << std::endl;

	m3 = m1 + m2;
	std::cout << m3(0,0) << std::endl;
	std::cout << m3(0,1) << std::endl;
	std::cout << m3(1,0) << std::endl;
	std::cout << m3(1,1) << std::endl;

    // lazy_vectors with 4 elements each.
    Vector v1( 4, 1.0 ), v2( 4, 2.0 ), v3( 4, 3.0 );

    // Add two vectors lazily and get the 2nd element.
    double d1 = ( v2 + v3 )( 2 );   // Look ma, no temporaries!
    std::cout << d1 << std::endl;

    // Subtract two vectors and add the result to a third vector.
    v1 += v2 - v3;                  // Still no temporaries!
    std::cout << '{' << v1(0) << ',' << v1(1)
              << ',' << v1(2) << ',' << v1(3) << '}' << std::endl;

    // This expression is disallowed because it does not conform
    // to the LazyVectorGrammar
    //(v2 + v3) += v1;

    return 0;
}



