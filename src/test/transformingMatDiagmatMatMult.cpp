/*
 * transformingMatDiagmatMatMult.cpp
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
	DLA::Matrix pre(3,3), post(3,3), result(3,3);
	DLA::DiagonalMatrix diag(3);

	pre(0,0) = 1.1; pre(0,1) = 1.2; pre(0,2) = 1.3;
	pre(1,0) = 2.1; pre(1,1) = 2.2; pre(1,2) = 2.3;
	pre(2,0) = 3.1; pre(2,1) = 3.2; pre(2,2) = 3.3;

	diag(0) = 1.0; diag(1) = 2.0; diag(2) = 3.0;

	post(0,0) = 1.0;  post(0,1) = 2.0; post(0,2) = 3.0;
	post(1,0) = 4.0;  post(1,1) = 5.0; post(1,2) = 6.0;
	post(2,0) = 7.0;  post(2,1) = 8.0; post(2,2) = 9.0;

	result = pre * diag * post;

	// The result shoule be :
	//  38.0   45.4   52.8
	//  68.0   81.4   94.8
	//  98.0  117.4  136.8
	std::cout << result(0,0) << " " << result(0,1) << " " << result(0,2) <<
			std::endl;
	std::cout << result(1,0) << " " << result(1,1) << " " << result(1,2) <<
			std::endl;
	std::cout << result(2,0) << " " << result(2,1) << " " << result(2,2) <<
			std::endl;

	return 0;
}




