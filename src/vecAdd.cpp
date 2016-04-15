/*
 * vecAdd.cpp
 *
 *  Created on: 2016/04/12
 *      Author: Masakatsu ITO
 */

#include "vecAdd.hpp"

namespace DLA = DenseLinAlg;

int main()
{


	DLA::ExpressionSyntaxChecker< DLA::LinAlgExprTrans >
		syntaxChecker =
				DLA::ExpressionSyntaxChecker< DLA::LinAlgExprTrans >();
    // proto::_default<> trans;

    // lazy_vectors with 3 elements each.
    DLA::Vector v1( 3, 1.0 ), v2( 3, 2.0 ), v3( 3, 3.0 );

    // Add two vectors lazily and get the 2nd element.
    std::cout << "Checking if v2 + v3 matches to LinAlgExprTrans rule ..."
    		<< std::endl;
    syntaxChecker( v2 + v3 );

    std::cout << "Checking if (v2 + v3)[2] matches to LinAlgExprTrans rule ..."
    		<< std::endl;
    syntaxChecker( ( v2 + v3 )( 2 ) );

    /* std::cout << "Checking if VecExprOpt()( ( v2 + v3 )( 2 ) )";
    std::cout << " matches to VecExprOpt rule ..." << std::endl;
    syntaxChecker( LinAlgExprTrans()( ( v2 + v3 )( 2 ) ) ); */

    // Look ma, no temporaries!
    double d1 = DLA::LinAlgExprTrans()( ( v2 + v3 )( 2 ) );
    // double d1 = trans( LinAlgExprTrans()( ( v2 + v3 )( 2 ) ) );
    std::cout << "DLA::LinAlgExprTrans()( ( v2 + v3 )( 2 ) ) = " <<
    		d1 << std::endl;

    // Subtract two vectors and add the result to a third vector.
    v1 += v2 - v3;                  // Still no temporaries!
    std::cout << "v1 += v2 - v3" << std::endl;
    std::cout << "v1 =";
    std::cout << '{' << v1(0) << ',' << v1(1)
              << ',' << v1(2) << ',' << v1(3) << '}' << std::endl;

    return 0;
}

