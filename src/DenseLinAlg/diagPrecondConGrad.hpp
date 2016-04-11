/*
 * diagPrecondConGrad.hpp
 *
 *  Created on: 2016/03/23
 *      Author: Masakatsu ITO
 */

#ifndef DENSELINALG_DIAGPRECONDCONGRAD_HPP_
#define DENSELINALG_DIAGPRECONDCONGRAD_HPP_

#include <math.h>

#include <iostream>

#include <DenseLinAlg/DenseLinAlg.hpp>
// #include <SparseLinAlg/SparseLinAlg.hpp>

namespace DLA = DenseLinAlg;
// namespace SLA = SparseLinAlg;

// invDiag = inverse matrix of ( diagonal part of coeff )
inline void makePreconditioner(double * invDiag, double** const coeff, int sz);


// resid = b - coeff * x
inline void vecMinusMatMultVec(double* resid,
		double * const b, double** const coeff, double * const x, int sz);

// z = invDiag * resid
inline void precondition( double* z,
		double * const invDiag, double * const resid, int sz);

// p = z
inline void vectorCopy( double* p, double * const z, int sz);

// q = coeff * p
inline void matMultVec( double* q,
		double** const coeff, double * const p, int sz);

// dot product of p and q
inline double dot( double * const p, double * const q, int sz);

// ans = initGuess + alpha * p
inline void vecPlusScalarMultVec( double * ans,
		double * const initGuess, double alpha, double * const p, int sz);


// ans += alpha * p
inline void assignAndPlusScalarMultVec( double* ans,
		double alpha, double * const p, int sz);


// absolute value of a vector b
inline double vecAbs( double * const b , int sz);


namespace DenseLinAlg {

	void diagPrecondConGrad( DLA::Vector & ansVec,
				const DLA::Matrix & coeffMat,
				const DLA::Vector & rhsVec,
				const DLA::Vector & initGuessVec,
				double convergenceCriterion)
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


#endif /* DENSELINALG_DIAGPRECONDCONGRAD_HPP_ */
