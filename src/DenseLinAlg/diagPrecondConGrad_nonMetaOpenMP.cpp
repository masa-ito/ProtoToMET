/*
 * diagPrecondConGrad_nonMetaOpenMP.cpp
 *
 *  Created on: 2016/02/08
 *      Author: Takaaki Miyajima and Masakatsu ITO
 */

#include <DenseLinAlg/diagPrecondConGrad.hpp>


// invDiag = inverse matrix of ( diagonal part of coeff )
inline void makePreconditioner(double * invDiag, double** const coeff, int sz)
{
	#pragma omp parallel for
	for (int i = 0; i < sz ; i++) invDiag[i] = 1.0 / coeff[i][i];
}

// resid = b - coeff * x
inline void vecMinusMatMultVec(double* resid,
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
inline void precondition( double* z,
		double * const invDiag, double * const resid, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) z[ri] = invDiag[ri] * resid[ri];
}

// p = z
inline void vectorCopy( double* p, double * const z, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) p[ri] = z[ri];
}

// q = coeff * p
inline void matMultVec( double* q,
		double** const coeff, double * const p, int sz)
{
#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++)	q[ri] = coeff[ri][0] * p[0];

#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++)
		for (int ci = 1; ci < sz; ci++)	q[ri] += coeff[ri][ci] * p[ci];
}

// dot product of p and q
inline double dot( double * const p, double * const q, int sz)
{
	double d = p[0] * q[0];
	#pragma omp parallel for reduction (+:d)
	for (int ri = 1; ri < sz; ri++) d += p[ri]*q[ri];
	return d;
}

// ans = initGuess + alpha * p
inline void vecPlusScalarMultVec( double * ans,
		double * const initGuess, double alpha, double * const p, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) ans[ri] = initGuess[ri] +  alpha * p[ri];
}

// ans += alpha * p
inline void assignAndPlusScalarMultVec( double* ans,
		double alpha, double * const p, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) ans[ri] += alpha * p[ri];
}


// absolute value of a vector b
inline double vecAbs( double * const b , int sz)
{
	double bSqr = b[0] * b[0];
	#pragma omp parallel for reduction (+:bSqr)
	for (int ri = 1; ri < sz; ri++) bSqr += b[ri] * b[ri];
	return sqrt( bSqr);
}

