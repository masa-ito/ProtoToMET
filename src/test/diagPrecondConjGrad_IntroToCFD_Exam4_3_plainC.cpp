/*
 * diagPrecondConjGrad_IntroToCFD_Exam4_3_plainC.cpp
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

#include "airCooledCylinder.hpp"

int main(int argc, char *argv[]) {

	int NumCtrlVol = 5, NumMeasurement = 1;
	if ( argc > 1 ) NumCtrlVol = atoi( argv[1] );
	std::cout << "The num. of grid points = " << NumCtrlVol << std::endl;

	if ( argc > 2 ) NumMeasurement = atoi( argv[2] );
	std::cout << "The num. of measurment = " << NumMeasurement << std::endl;

	printConstants();

	double elapsedTimeSum = 0.0;

	for (int iM = 0; iM < NumMeasurement; iM++ ) {

		DLA::Matrix coeffMat( NumCtrlVol, NumCtrlVol, 0.0);

		double deltaX = CylinderLength / NumCtrlVol,
				deltaDirichlet = deltaX / 2.0;

		const double scale = - ThermalConductivity * Area;
		if ( NumMeasurement < 2 ) {
			std::cout << "scale = - ThermalConductivity * Area" <<
				std::endl;
			std::cout << "= " << scale << std::endl;
		}

		const double nSqr =  ConvectiveHeatTransCoeff * Circumference /
							( ThermalConductivity * Area );

		coeffMat(0,0) = ( 1.0 / deltaDirichlet // Dirichlet condition term
						  + 1.0 / deltaX + nSqr * deltaX ) * scale;

		coeffMat( NumCtrlVol-1, NumCtrlVol-1) =
				( 2.0 / deltaX
				  - 1.0 / deltaX // Neumann condition term
				  + nSqr * deltaX ) * scale;

		coeffMat(0, 1) = - 1.0 / deltaX * scale;
		coeffMat( NumCtrlVol-1, NumCtrlVol-2) = - 1.0 / deltaX * scale;

		int i;
		for (i = 1; i < NumCtrlVol-1; i++) {
			coeffMat( i, i-1) = - 1.0 / deltaX * scale;
			coeffMat( i, i) = ( 2.0 / deltaX + nSqr * deltaX) * scale;
			coeffMat( i, i+1) = - 1.0 / deltaX * scale;
		}

		DLA::Vector rhsVec( NumCtrlVol);

		rhsVec(0) = ( 2.0 / deltaX * HotTemperature // Dirichlet condition
					  + nSqr * deltaX * AmbientTemperature ) * scale;
		for (i = 1; i < NumCtrlVol; i++)
			rhsVec( i) = nSqr * deltaX * AmbientTemperature * scale;

		if ( NumMeasurement < 2 ) {
			printCoefficients( coeffMat, scale);
			printRHS( rhsVec, scale);
		}

		SLA::DiagonalPreconditioner precond( coeffMat);
		SLA::ConjugateGradient< DLA::Matrix, SLA::DiagonalPreconditioner >
													cg( coeffMat, precond);

		const DLA::Vector tempGuess( NumCtrlVol, (100.0 + 20.0) / 2.0);
		const double convergenceCriterion = 1.0e-7;
		// const int maxIter = 100;

		DLA::Vector temperature( NumCtrlVol);
		// temperature = cg.solve(rhsVec, tempGuess, convergenceCriterion);

		// Measuring the elapsed time of our conjugate gradient procedure
		auto start = std::chrono::system_clock::now();

		DLA::diagPrecondConGrad( temperature,
				coeffMat, rhsVec, tempGuess, convergenceCriterion);

		auto end = std::chrono::system_clock::now();
		auto diff = end - start;

		elapsedTimeSum +=
			double( std::chrono::duration_cast<std::chrono::milliseconds>
														(diff).count() );

		if ( NumMeasurement < 2 )
			printCalculatedAndExactTemperatureDistributions< DLA::Vector >(
															temperature);
	}

	std::cout << std::endl;
	std::cout << "elapsed time of conjugate gradient = "
	  << elapsedTimeSum / NumMeasurement
	  << " msec."
	  << std::endl;

	return 0;
}


