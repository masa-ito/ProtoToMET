/*
 * matVecMultAndVecSubMetaOpenMP.cpp
 *
 *  Created on: 2016/05/16
 *      Author: Masakatsu ITO
 */

#ifdef _OPENMP
#include <ParallelizationTypeTag/OpenMP.hpp>
#endif

#include "matVecMultAndVecSubMetaOpenMP.hpp"


// namespace DLA = DenseLinAlg;

// void testMatVecMultAndVecAdd()
int main()
{
	using namespace DenseLinAlg;

    Matrix matA( 3, 3);
    Vector vecX(3), vecB(3), vecBminusX(3), vecR(3);

    matA(0,0) = 1.00; matA(0,1) = 1.01; matA(0,2) = 1.02;
    matA(1,0) = 1.10; matA(1,1) = 1.11; matA(1,2) = 1.12;
    matA(2,0) = 1.20; matA(2,1) = 1.21; matA(2,2) = 1.22;

    vecX(0) = 1.0;
    vecX(1) = 2.0;
    vecX(2) = 3.0;

    vecB(0) = 4.0;
    vecB(1) = 5.0;
    vecB(2) = 6.0;

	//GrammarChecker< ExprGrammar >
	//	checker = GrammarChecker< ExprGrammar >();

    // Vector object alons is not an expression template.
	// checker( vecB );

    std::cout
		<< "Checking if (vecB - vecX) matches to VecMapGrammar ..."
        << std::endl;
    GrammarChecker< VecMapGrammar >()( vecB - vecX );

    std::cout
		<< "Checking if (vecB - vecX)(2) matches to VecMapGrammar  ..."
        << std::endl;
    GrammarChecker< VecMapGrammar >()( ( vecB - vecX )(2) );


    std::cout
		<< "Checking if (matA * vecX) matches to VecMapReduceGrammar ..."
        << std::endl;
    GrammarChecker< VecMapReduceGrammar >()( matA * vecX );

    std::cout
		<< "Checking if (matA * vecX)(2) matches to VecMapReduceGrammar..."
        << std::endl;
    GrammarChecker< VecMapReduceGrammar >()( ( matA * vecX )(2) );

    // double elm2 = VecExprGrammar()( ( matA * vecX)(2) );
    // std::cout << "( matA * vecX)(2) = " << elm2 << std::endl;


    std::cout
		<< "Checking if (vecB - matA * vecX) "
		<< "matches to VecMapReduceGrammar ..."
        << std::endl;
    GrammarChecker< VecMapReduceGrammar >()( vecB - matA * vecX );

    std::cout << "Checking if ( vecB - matA * vecX)(2) "
    		<< "matches to VecMapReduceGrammar ..."
        	<< std::endl;
    GrammarChecker< VecMapReduceGrammar >()( ( vecB - matA * vecX)(2) );

    double elm2 = VecExprGrammar()( ( vecB - matA * vecX)(2) );
    std::cout << "( vecB - matA * vecX)(2) = " << elm2 << std::endl;
    // This should be  -1.28 . */

    vecBminusX = vecB - vecX;
    std::cout << " vecBminusX = vecB - vecX = " << std::endl;
    std::cout << " ( " << vecBminusX(0) << ", " << vecBminusX(1) << ", " <<
    		vecBminusX(2) << ")" << std::endl;
    // This should be ( 3.0 , 3.0 , 3.0) . */



    // vecR = matA * vecX;
    vecR = vecB - matA * vecX;

    // std::cout << " vecR = vecB - vecX = " << std::endl;
    // std::cout << " vecR = matA * vecX = " << std::endl;
    std::cout << " vecR = vecB - matA * vecX = " << std::endl;
    std::cout << " ( " << vecR(0) << ", " << vecR(1) << ", " <<
    		vecR(2) << ")" << std::endl;
    // This should be ( -2.08 , -1.68 , -1.28) . */

	return 0;
}

