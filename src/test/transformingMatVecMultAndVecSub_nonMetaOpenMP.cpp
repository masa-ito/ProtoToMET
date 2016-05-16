/*
 * transformingMatVecMultAndVecSub_nonMetaOpenMP.cpp
 *
 *  Created on: 2016/05/16
 *      Author: Masakatsu ITO
 */

#include <ParallelizationTypeTag/OpenMP.hpp>

#include "transformingMatVecMultAndVecSub.hpp"

int main()
{
	testMatVecMultAndVecSub();

	return 0;
}

