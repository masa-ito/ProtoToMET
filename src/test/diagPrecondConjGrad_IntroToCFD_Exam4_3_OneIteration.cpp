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

void printVec(const DLA::Vector& vec, const char* vecName)
{
	std::cout << vecName << " = " << std::endl;
	for (int i = 0; i < vec.columnSize(); i++)
		std::cout << vec(i) << std::endl;
	std::cout << std::endl;
}


template < class Preconditioner >
void conjugateGradientOneIteration( const DLA::Vector& b,
		const DLA::Vector iniGuess, const DLA::Matrix& coeff,
		const Preconditioner& precond,
		const double convergenceCriterion,
		DLA::Vector& lhs)
{
	DLA::Vector resid( b.columnSize()), z( b.columnSize()),
						q( b.columnSize());

	resid = b - coeff * iniGuess;

	printVec( resid, "resid");
	/* octave:8> resid = b - coeff * iniGuess
	resid =

	   200
	  -200
	  -200
	  -200
	  -200 */

	z = precond.solve( resid);
	printVec( z, "z");
	/* octave:14> z = precond * resid
	z =

	   10.000
	  -13.333
	  -13.333
	  -13.333
	  -20.000 */

	double rho = resid.dot( z);
	std::cout << "rho = " << rho << std::endl;
	/* octave:15> rho = resid' * z
	rho =  14000 */

	DLA::Vector p = z;
	q = coeff * p;
	printVec(q, "q");
	/* octave:17> q = coeff * p
	q =

	   266.667
	  -183.333
	   -66.667
	   -33.333
	  -133.333 */
	double alpha = rho / p.dot(q);
	std::cout << "alpha = " << alpha << std::endl;
	/* octave:18> alpha = rho / (p' * q )
	alpha =  1.5366 */

	lhs = iniGuess + alpha * p;
	printVec(lhs, "lhs");
	/* octave:19> lhs = iniGuess + alpha * p
	lhs =

	   75.366
	   39.512
	   39.512
	   39.512
	   29.268 */
	resid -= alpha * q;
	printVec(resid, "resid");
	/* octave:20> resid -= alpha * q
	resid =

	  -209.7561
	    81.7073
	   -97.5610
	  -148.7805
	     4.8780 */

	const double bAbs = b.abs();

	// iterative part
	z = precond.solve( resid);
	double prevRho = rho;
	rho = resid.dot( z);

	double beta = rho / prevRho;
	p = z + beta * p;

	q = coeff * p;
	alpha = rho / p.dot(q);
	lhs += alpha * p;
	resid -= alpha * q;

}


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
	/* SLA::ConjugateGradient< DLA::Matrix, SLA::DiagonalPreconditioner >
	                   							cg( coeffMat, precond); */

	const DLA::Vector tempGuess( NumCtrlVol, (100.0 + 20.0) / 2.0);
	const double convergenceCriterion = 1.0e-7;
	// const int maxIter = 100;

	DLA::Vector temperature( NumCtrlVol);
	conjugateGradientOneIteration(rhsVec, tempGuess, coeffMat, precond,
		convergenceCriterion, temperature);

	printVec( temperature, "temperature");
	/* 67.0052
	   40.5926
	   26.4985
	   22.4716
	   21.8284 */

	return 0;

}


