/*
 * transformingMatrixVectorMultiplication.cpp
 *
 *  Created on: 2015/09/30
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

#include <DenseLinAlg/DenseLinAlg.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;


void testVecAdd()
{
	using namespace DenseLinAlg;

    // lazy_vectors with 3 elements each.
    Vector v1( 3, 1.0 ), v2( 3, 2.0 ), v3( 3, 3.0 );

	GrammarChecker< ExprGrammar >
		checker = GrammarChecker< ExprGrammar >();

    // Add two vectors lazily and get the 2nd element.
    std::cout << "Checking if v2 + v3 matches to ExprGrammar ..."
    		<< std::endl;
    checker( v2 + v3 );

    std::cout << "Checking if (v2 + v3)(2) matches to ExprGrammar ..."
    		<< std::endl;
    checker( ( v2 + v3 )( 2 ) );

    // std::cout << "Checking if VecExprOpt()( ( v2 + v3 )( 2 ) )";
    // std::cout << " matches to VecExprOpt rule ..." << std::endl;
    // checker( ExprGrammar()( ( v2 + v3 )( 2 ) ) );

    // proto::_default<> trans;

    // Look ma, no temporaries!
    double d1 = ExprGrammar()( ( v2 + v3 )( 2 ) );
    // double d1 = trans( ExprGrammar()( ( v2 + v3 )( 2 ) ) );
    std::cout << "( v2 + v3 )( 2 ) = " << d1 << std::endl;

    // Subtract two vectors and add the result to a third vector.
    v1 += v2 - v3;                  // Still no temporaries!
    std::cout << "v1 += v2 - v3" << std::endl;
    std::cout << "v1 =";
    std::cout << '{' << v1(0) << ',' << v1(1)
              << ',' << v1(2) << ',' << v1(3) << '}' << std::endl;

    return;
}

void testMatAdd()
{
	using namespace DenseLinAlg;

	Matrix m1( 3, 3), m2( 3, 3), m3( 3, 3);
	for (int ri=0; ri < 3; ri++)
		for (int ci=0; ci < 3; ci++)
		{
			m1(ri, ci) = 1.0 + 0.1 * ri + 0.01 * ci;
			m2(ri, ci) = 2.0 + 0.1 * ri + 0.01 * ci;
		}

	GrammarChecker< ExprGrammar >
		checker = GrammarChecker< ExprGrammar >();

	std::cout << "Checking if m1 + m2 matches to ExprGrammar ..."
			<< std::endl;
	checker( m1 + m2 );

	std::cout << "Checking if (m1 + m2)(0,1) matches to ExprGrammar ..."
			<< std::endl;
	checker( (m1 + m2)(0,1) );

    // proto::_default<> trans;
	double elm01 = ExprGrammar()( (m1 + m2)(0,1) );
	// double elm01 = trans( ExprGrammar()( (m1 + m2)(0,1) ) );
	std::cout << " (m1 + m2)(0,1) = " << elm01 << std::endl;

	m3 = m1 + m2;

	//  !!! these should be modified !!!!!
	std::cout << "m3 = " << std::endl;
	std::cout << "( ( " << m3(0,0) << "    " << m3(0,1) << " "
			<< m3(0,2) << " ) " << std::endl;
	std::cout << "  ( " << m3(1,0) << "  " << m3(1,1) << " " <<
			m3(1,2) << " ) " << std::endl;
	std::cout << "  ( " << m3(2,0) << "  " << m3(2,1) << " " <<
			m3(2,2) << " ) )" << std::endl;

	return;
}

void testMatVecMul()
{
	using namespace DenseLinAlg;

    Matrix mat( 3, 3);
    Vector vec1(3), vec2(3);

    mat(0,0) = 1.00; mat(0,1) = 1.01; mat(0,2) = 1.02;
    mat(1,0) = 1.10; mat(1,1) = 1.11; mat(1,2) = 1.12;
    mat(2,0) = 1.20; mat(2,1) = 1.21; mat(2,2) = 1.22;

    vec1(0) = 1.0;
    vec1(1) = 2.0;
    vec1(2) = 3.0;

	GrammarChecker< ExprGrammar >
		checker = GrammarChecker< ExprGrammar >();

    std::cout << "Checking if mat * vec1 matches to ExprGrammar ..."
        	<< std::endl;
	checker( mat * vec1 );

    /* std::cout << "Checking if MatVecMultGrammar()( mat * vec1) "
    		<< "matches to ExprGrammar ..."
        	<< std::endl;
	checker( MatVecMultGrammar()( mat * vec1) ); */

    std::cout << "Checking if ( mat * vec1)(2) matches to ExprGrammar ..."
        	<< std::endl;
	checker( ( mat * vec1)(2) );

    /* std::cout << "Checking if VecExprGrammar()( ( mat * vec1)(2) ) "
    		<< "matches to ExprGrammar ..."
        	<< std::endl;
	checker( VecExprGrammar()( ( mat * vec1)(2) ) ); */

    // proto::_default<> trans;
    double elm2 = VecExprGrammar()( ( mat * vec1)(2) );
    // double elm2 = trans( VecExprGrammar()( ( mat * vec1)(2) ) );

    std::cout << "( mat * vec1)(2) = " << elm2 << std::endl;
    // This should be 7.28 .

    vec2 = mat * vec1;

    std::cout << " vec2 = mat * vec1 = " << std::endl;
    std::cout << " ( " << vec2(0) << ", " << vec2(1) << ", " <<
    		vec2(2) << ")" << std::endl;
    // This should be ( 6.08 , 6.68 , 7.28) .

	return;
}

int main()
{
    // std::cout << "Let's see if any temporary oject is copied !" << std::endl;

	testVecAdd();
	testMatAdd();
	testMatVecMul();

    return 0;
}

