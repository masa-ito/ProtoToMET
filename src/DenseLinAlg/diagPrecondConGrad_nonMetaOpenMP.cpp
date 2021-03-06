/*
 * diagPrecondConGrad_nonMetaOpenMP.cpp
 *
 *  Created on: 2016/02/08
 *      Author: Takaaki Miyajima and Masakatsu ITO
 */

#include <DenseLinAlg/diagPrecondConGrad.hpp>


// invDiag = inverse matrix of ( diagonal part of coeff )
inline void makePreconditioner(double * const invDiag, double** const coeff, int sz)
{
	#pragma omp parallel for
	for (int i = 0; i < sz ; i++) invDiag[i] = 1.0 / coeff[i][i];
}

// resid = b - coeff * x
inline void vecMinusMatMultVec(double* resid,
		double * const b, double** const coeff, double * const x, int sz)
{
	int ri, ci;
	double matVec;

	#pragma omp parallel for \
	 	 	 private( ri, ci, matVec) shared( sz, resid)
	// double * const resid cannot be put in to share( ).
	// http://stackoverflow.com/questions/13199398/
	//            openmp-predetermined-shared-for-shared
	for (ri = 0; ri < sz; ri++) {
		matVec = coeff[ri][0] * x[0];
		for (ci = 1; ci  < sz; ci++) matVec += coeff[ri][ci] * x[ci];
		resid[ri] = b[ri] - matVec;
	}
}

// z = invDiag * resid
inline void precondition( double * const z,
		double * const invDiag, double * const resid, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) z[ri] = invDiag[ri] * resid[ri];
}

// p = z
inline void vectorCopy( double * const p, double * const z, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) p[ri] = z[ri];
}

// q = coeff * p
inline void matMultVec( double * q,
		double** const coeff, double * const p, int sz)
{
	int ri, ci;
	double matVec;

	#pragma omp parallel for default(none) \
	            private( ri, ci, matVec) shared( sz, q)
	// double * const q cannot be put into shared( ).
	// http://stackoverflow.com/questions/13199398/
	//            openmp-predetermined-shared-for-shared
	for (ri = 0; ri < sz; ri++) {
		matVec = coeff[ri][0] * p[0];
		for (ci = 1; ci < sz; ci++)	matVec += coeff[ri][ci] * p[ci];
		q[ri] = matVec;
	}
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
inline void vecPlusScalarMultVec( double * const ans,
		double * const initGuess, double alpha, double * const p, int sz)
{
	#pragma omp parallel for
	for (int ri = 0; ri < sz; ri++) ans[ri] = initGuess[ri] +  alpha * p[ri];
}

// ans += alpha * p
inline void assignAndPlusScalarMultVec( double* const ans,
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

