/*
 * diagPrecondConjGrad_IntroToCFD_Exam4_3.cpp
 *
 * ref) H. K. Versteeg and W. Malalasekera,
 *     "An Introduction  to Computational Fluid Dynamics,
 *     The Finite Volume Method", 2nd Ed.
 *     Pearson Educational Limited 1995, 2007.
 *
 *     Example 4.3
 *
 *  Created on: 2016/01/27
 *      Author: Masakatsu ITO
 */

#ifdef _OPENMP
#include <ParallelizationTypeTag/OpenMP.hpp>
#endif

#include <math.h>

#include <iostream>
#include <boost/proto/proto.hpp>

#include <DenseLinAlg/DenseLinAlg.hpp>
#include <SparseLinAlg/SparseLinAlg.hpp>

namespace DLA = DenseLinAlg;
namespace SLA = SparseLinAlg;


int main()
{
	const int NumCtrlVol = 5;

	DLA::Matrix coeffMat( NumCtrlVol, NumCtrlVol, 0.0);

	coeffMat(0,0) =  20.0; coeffMat(0,1) =  -5.0;
	coeffMat(1,0) =  -5.0; coeffMat(1,1) =  15.0; coeffMat(1,2) =  -5.0;
	coeffMat(2,1) =  -5.0; coeffMat(2,2) =  15.0; coeffMat(2,3) =  -5.0;
	coeffMat(3,2) =  -5.0; coeffMat(3,3) =  15.0; coeffMat(3,4) =  -5.0;
	                       coeffMat(4,3) =  -5.0; coeffMat(4,4) =  10.0;

	DLA::Vector rhsVec( NumCtrlVol);
	rhsVec(0) = 1100.0; rhsVec(1) = 100.0; rhsVec(2) = 100.0;
	rhsVec(3) = 100.0; rhsVec(4) = 100.0;

	SLA::DiagonalPreconditioner precond( coeffMat);
	SLA::ConjugateGradient< DLA::Matrix, SLA::DiagonalPreconditioner >
	                   							cg( coeffMat, precond);

	const DLA::Vector tempGuess( NumCtrlVol, (100.0 + 20.0) / 2.0);
	const double convergenceCriterion = 1.0e-7;
	const int maxIter = 100;

	DLA::Vector temperature( NumCtrlVol);
	temperature = cg.solve(rhsVec, tempGuess, convergenceCriterion, maxIter);

	std::cout << temperature(0) << std::endl;
	std::cout << temperature(1) << std::endl;
	std::cout << temperature(2) << std::endl;
	std::cout << temperature(3) << std::endl;
	std::cout << temperature(4) << std::endl;

	return 0;
}


