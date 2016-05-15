/*
 * OpenMP.hpp
 *
 *  Created on: 2016/05/15
 *      Author: Masakatsu ITO
 */

#ifndef PARALLELIZATIONTYPETAG_HPP_
#define PARALLELIZATIONTYPETAG_HPP_

#include <ParallelizationTypeTag/ParallelizationTypeTag.hpp>

namespace ParallelizationTypeTag {

	typedef SingleProcess< OpenMP< NoSIMD > > Specified;

}


#endif /* PARALLELIZATIONTYPETAG_HPP_ */
