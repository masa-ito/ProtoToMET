/*
 * protoDeepCopyMatrixVectorExpr.cpp
 *
 *  Created on: 2015/12/08
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

#include <DenseLinAlg/DenseLinAlg.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

namespace DLA = DenseLinAlg;

int main()
{
    DLA::Matrix matA( 3, 3);
    DLA::Vector vecX(3), vecB(3), vecR(3);

    matA(0,0) = 1.00; matA(0,1) = 1.01; matA(0,2) = 1.02;
    matA(1,0) = 1.10; matA(1,1) = 1.11; matA(1,2) = 1.12;
    matA(2,0) = 1.20; matA(2,1) = 1.21; matA(2,2) = 1.22;

    vecX(0) = 1.0;
    vecX(1) = 2.0;
    vecX(2) = 3.0;

    vecB(0) = 4.0;
    vecB(1) = 5.0;
    vecB(2) = 6.0;

	DLA::GrammarChecker< DLA::ExprGrammar >
		checker = DLA::GrammarChecker< DLA::ExprGrammar >();

	auto expr = proto::deep_copy( vecB - matA * vecX );

    std::cout << "Checking if (vecB - matA * vecX) matches to ExprGrammar ..."
        	<< std::endl;
	checker( expr );

    std::cout << "Checking if ( vecB - matA * vecX)(2) "
    		<< "matches to ExprGrammar ..."
        	<< std::endl;
	checker( expr(2) );

    proto::_default<> trans;
    double elm2 = trans( DLA::VecExprGrammar()( expr(2) ) );

    std::cout << "( vecB - matA * vecX)(2) = " << elm2 << std::endl;
    // This should be  -1.28 .

    vecR = expr;

    std::cout << " vecR = vecB - matA * vecX = " << std::endl;
    std::cout << " ( " << vecR(0) << ", " << vecR(1) << ", " <<
    		vecR(2) << ")" << std::endl;
    // This should be ( -2.08 , -1.68 , -1.28) .


	return 0;
}

