/*
 * transformingDiagMatVecMult.cpp
 *
 *  Created on: 2016/02/04
 *      Author: Masakatsu ITO
 */

#include <iostream>
#include <boost/proto/proto.hpp>

#include <DenseLinAlg/DenseLinAlg.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

#include <iostream>
#include <boost/proto/proto.hpp>

#include <DenseLinAlg/DenseLinAlg.hpp>

namespace mpl = boost::mpl;
namespace proto = boost::proto;

namespace DLA = DenseLinAlg;

int main()
{
	DLA::Vector vec(3), result(3);
	vec(0) = 1.1; vec(1) = 2.2; vec(2) = 3.3;

	DLA::DiagonalMatrix diag(3);
	diag(0) = 1.0; diag(1) = 2.0; diag(2) = 3.0;

	result = diag * vec;

	// The result should be ( 1.1, 4.4, 9.9).
	std::cout << result(0) << " " << result(1) << " " << result(2) <<
			std::endl;

	return 0;
}


