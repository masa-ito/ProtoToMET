/*
 * adaptingArrayWrapper2x2.cpp
 *
 *  Created on: 2015/05/28
 *      Author: Masakatsu ITO
 */

// The original version is at
// http://www.kmonos.net/alang/boost/classes/proto.html
//   Presented by k.inaba (kiki .a.t. kmonos.net)
//   under Creative Commons 0 1.0.
//  http://creativecommons.org/publicdomain/zero/1.0/

#include <iostream>
#include <boost/proto/proto.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


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
	// Matrix data are stored in an auto array.
	//
	class Matrix
	{
	private:
		double m[2][2];
	public:
		Matrix(){ std::cout << "Created" << std::endl;}
		Matrix(const Matrix&){ std::cout << "Copied" << std::endl;}


		// accesing to a matrix element
		double& operator()(int i, int j) { return m[i][j]; }
		const double& operator()(int i, int j) const { return m[i][j]; }

		// assigning the lhs of a matrix expression into this matrix
		template<typename Expr>
		Matrix& operator=( const Expr& expr )
		{
			for(int i=0; i<2; ++i)
				for(int j=0; j<2; ++j) {
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

int main()
{
	ETSample::Matrix m;
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



