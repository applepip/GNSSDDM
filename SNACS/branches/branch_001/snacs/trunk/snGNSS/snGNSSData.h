/*
 * snGNSSData.h
 *
 *  Created on: Mar 25, 2010
 *      Author: frank
 */

#ifndef SNGNSSDATA_H_
#define SNGNSSDATA_H_

#include <vector>
#include <iostream>

/*
 * \brief class for generation of navigation data
 */
class snGNSSData {
public:
	enum data_type {
		NONE, RANDOM
	};

	snGNSSData(data_type _dt, double _len, double _bps);
	virtual ~snGNSSData();

	double get_value_for_time(double t) {
		const int index = floor(t * bps);
		//std::cout << "d" << index << ", ";
		//std::cout.flush();
		return data_stream.at(index);
	}

private:
	double len, bps;
	data_type dt;
	std::vector<double> data_stream;
};

#endif /* SNGNSSDATA_H_ */
