/*
 * transformingMatrixWithHeapArray.cpp
 *
 *  Created on: 2015/09/28
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


class Matrix;

// from transformingVectorOnHeapArray.cpp

struct IndxDist : proto::or_<
	proto::when< proto::terminal< Matrix>,
				proto::_make_subscript( proto::_, proto::_state) >,
	proto::terminal< Matrix >,
	proto::plus< IndxDist, IndxDist> ,
	proto::minus< IndxDist, IndxDist>
> {};

// This grammar describes which vector expressions
// are allowed.
struct VecExprOpt : proto::or_<
	proto::when< proto::subscript< IndxDist, proto::_ >,
				IndxDist(proto::_left, proto::_right) >,
	proto::terminal< Vector >,
	proto::plus< VecExprOpt, VecExprOpt> ,
	proto::minus< VecExprOpt, VecExprOpt>
> {};


// The above grammar is associated with this domain.
template<typename Expr> struct VecExpr;
struct VecDomain
	: proto::domain<proto::generator<VecExpr>, VecExprOpt> {};


//
// Vector Expression Templates
//
template<typename Expr>
struct VecExpr
	: proto::extends<Expr, VecExpr<Expr>, VecDomain> {
		explicit VecExpr(const Expr& e)
			: proto::extends<Expr, VecExpr<Expr>, VecDomain>(e) {
		}
};


// from adaptingMatrixWithHeapArray.cpp **************************************

namespace ETSample
{
	//
	// Syntax for Expression Templates, terminal value or addition
	// Grammar ::= _ | Grammar + Grammar
	//
	struct Grammar
		: proto::or_<
			proto::terminal<proto::_>,
			proto::plus<Grammar, Grammar>
		>{};

	//
	// The above grammar is associated with this domain.
	//
	template<typename Expr> struct MatrixExpr;
	struct MatrixDomain
		: proto::domain<proto::generator<MatrixExpr>, Grammar> {};

	//
	// Matrix Expression Templates
	//
	template<typename Expr>
	struct MatrixExpr
		: proto::extends<Expr, MatrixExpr<Expr>, MatrixDomain>
	{
		explicit MatrixExpr(const Expr& e)
			: proto::extends<Expr, MatrixExpr<Expr>, MatrixDomain>(e) {}
	};

	//
	// Context for evaluating an element of matrix expressions
	//
	struct EvalCtxByIndex
		: proto::callable_context<const EvalCtxByIndex>
	{
		typedef double result_type;

		int i_, j_;
		EvalCtxByIndex(int i, int j) :  i_(i), j_(j) {}

		// matrix element
		template<typename Matrix>
		double operator()(proto::tag::terminal, const Matrix& data) const
		{
			return data(i_,j_);
		}

		// addition of matrix expression terms
		template<typename E1, typename E2>
		double operator()(proto::tag::plus, const E1& e1, const E2& e2) const
		{
			return proto::eval(e1, *this) + proto::eval(e2, *this);
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
				for(int j=0; colSize; ++j) {
					// evaluating the (i,j) element of a matrix expression
					const EvalCtxByIndex ctx(i, j);
					m[i][j] = proto::eval(proto::as_expr<MatrixDomain>(expr), ctx);
				}
			return *this;
		}
	};

	// Adapting Matrix class to Proto
	template<typename> struct IsMatrix : mpl::false_ {};
	template<> struct IsMatrix<Matrix> : mpl::true_  {};
	BOOST_PROTO_DEFINE_OPERATORS(IsMatrix, MatrixDomain)
}

int main() {
	ETSample::Matrix m( 2, 2, 0.0);
	m(0,0) = 1;
	m(0,1) = 2;
	m(1,0) = 3;
	m(1,1) = 4;
	m = m+m+m;
	std::cout << m(0,0) << std::endl;
	std::cout << m(0,1) << std::endl;
	std::cout << m(1,0) << std::endl;
	std::cout << m(1,1) << std::endl;
}




