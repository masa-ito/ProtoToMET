/*
 * diagPrecondConGrad_nonMetaOpenMP.cpp
 *
 *  Created on: 2016/02/08
 *      Author: Takaaki Miyajima and Masakatsu ITO
 */

#include <math.h>

#include <iostream>

#include <ParallelizationTypeTag/ParallelizationTypeTag.hpp>

#include <DenseLinAlg/DenseLinAlg.hpp>
// #include <SparseLinAlg/SparseLinAlg.hpp>

// #include <DenseLinAlg/diagPrecondConGrad.hpp>

// namespace DLA = DenseLinAlg;
// namespace SLA = SparseLinAlg;

// invDiag = inverse matrix of ( diagonal part of coeff )
void makePreconditioner(double * invDiag, double** const coeff, int sz)
{
	#pragma omp parallel for
	for (int i = 0; i < sz ; i++) invDiag[i] = 1.0 / coeff[i][i];
}

// resid = b - coeff * x
void vecMinusMatMultVec(double* resid,
		double * const b, double** const coeff, double * const x, int sz)
{
	for (int ri = 0; ri < sz; ri++) {
		double p = coeff[ri][0] * x[0];
		#pragma omp parallel for reduction (+:p)
		for (int ci = 1; ci  < sz; ci++) p += coeff[ri][ci] * x[ci];
		resid[ri] = b[ri] - p;
	}
}

// z = invDiag * resid
void precondition( double* z,
		double * const invDiag, double * const resid, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) z[ri] = invDiag[ri] * resid[ri];
}

// p = z
void vectorCopy( double* p, double * const z, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) p[ri] = z[ri];
}

// q = coeff * p
void matMultVec( double* q,
		double** const coeff, double * const p, int sz)
{
#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++)	q[ri] = coeff[ri][0] * p[0];

#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++)
		for (int ci = 1; ci < sz; ci++)	q[ri] += coeff[ri][ci] * p[ci];
}

// dot product of p and q
double dot( double * const p, double * const q, int sz)
{
	double d = p[0] * q[0];
	#pragma omp parallel for reduction (+:d)
	for (int ri = 1; ri < sz; ri++) d += p[ri]*q[ri];
	return d;
}

// ans = initGuess + alpha * p
void vecPlusScalarMultVec( double * ans,
		double * const initGuess, double alpha, double * const p, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) ans[ri] = initGuess[ri] +  alpha * p[ri];
}

// ans += alpha * p
void assignAndPlusScalarMultVec( double* ans,
		double alpha, double * const p, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) ans[ri] += alpha * p[ri];
}


// absolute value of a vector b
double vecAbs( double * const b , int sz)
{
	double bSqr = b[0] * b[0];
	#pragma omp parallel for reduction (+:bSqr)
	for (int ri = 1; ri < sz; ri++) bSqr += b[ri] * b[ri];
	return sqrt( bSqr);
}

namespace DenseLinAlg {

	namespace PAR = ParallelizationTypeTag;

	template <>
	void diagPrecondConGrad(
		Vector< PAR::OpenMP > & ansVec,
		const Matrix< PAR::OpenMP> & coeffMat,
		const Vector< PAR::OpenMP > & rhsVec,
		const Vector< PAR::OpenMP > & initGuessVec,
		double convergenceCriterion
	)
	{
		double* ans = ansVec.data;
		double** const coeff = coeffMat.m;
		double* const b = rhsVec.data;
		double* initGuess = initGuessVec.data;
		const int sz = rhsVec.sz;

		double *invDiag = new double[ sz],
			*resid = new double[ sz],
			*z = new double[ sz],
			*q = new double[ sz],
			*p = new double[ sz];


		makePreconditioner(invDiag, coeff, sz);

		vecMinusMatMultVec( resid, b, coeff, initGuess, sz);
		precondition( z, invDiag, resid, sz);
		double rho = dot( resid, z, sz);

		vectorCopy( p, z, sz);
		matMultVec( q, coeff, p, sz);
		double alpha = rho / dot( p, q, sz);

		vecPlusScalarMultVec( ans, initGuess, alpha, p, sz);
		assignAndPlusScalarMultVec( resid, - alpha, q, sz);

		const double bAbs = vecAbs( b , sz);

		while ( vecAbs( resid, sz) / bAbs > convergenceCriterion )
		{
			precondition( z, invDiag, resid, sz);
			double prevRho = rho;
			rho = dot( resid, z, sz);

			double beta = rho / prevRho;
			vecPlusScalarMultVec( p, z, beta, p, sz);

			matMultVec( q, coeff, p, sz);
			alpha = rho / dot( p, q, sz);
			assignAndPlusScalarMultVec( ans, alpha, p, sz);
			assignAndPlusScalarMultVec( resid, -alpha, q, sz);
		}

		delete [] p;
		delete [] q;
		delete [] z;
		delete [] resid;
		delete [] invDiag;

		return;
	}

}
