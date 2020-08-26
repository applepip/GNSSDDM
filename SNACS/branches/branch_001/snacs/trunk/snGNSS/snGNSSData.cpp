/*
 * snGNSSData.cpp
 *
 *  Created on: Mar 25, 2010
 *      Author: frank
 */
#include <cmath>
#include <iostream>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/bernoulli_distribution.hpp>
#include <ctime> //std::time
#include "snGNSSData.h"

typedef boost::mt19937 base_generator_type;

snGNSSData::snGNSSData(data_type _dt, double _len, double _bps) :
	len(_len), bps(_bps), dt(_dt) {

	size_t data_bits = ceil(len * bps) + 1;
//	std::cout << "generating " << data_bits << " data bits.\n";
	switch (dt) {
	case NONE:
		data_stream.resize(data_bits);
		for (size_t i = 0; i < data_stream.size(); i++) {
			data_stream.at(i) = 1.0;
		}
		break;
	case RANDOM:
		data_stream.resize(data_bits);

		// initialize random number generator:
		base_generator_type random_number_generator;

		unsigned int seed_time_preset = std::time(0);
		random_number_generator.seed(seed_time_preset);
		//Sig.results_file->write("snNoise/parameters/seed_time_preset", seed_time_preset);

		static boost::bernoulli_distribution<double> bernoulli_dist(0.5);
		static boost::variate_generator<base_generator_type&, boost::bernoulli_distribution<double> >
				bernoulli_value(random_number_generator, bernoulli_dist);

		for (size_t i = 0; i < data_stream.size(); i++) {
			if (bernoulli_value() == true) {
				data_stream.at(i) = 1.0;
			} else {
				data_stream.at(i) = -1.0;
			}
//			std::cout << "bit " << i << ": " << data_stream.at(i) << ", ";
		}
//		std::cout << "\n";
		break;
	}
}

snGNSSData::~snGNSSData() {
	// TODO Auto-generated destructor stub
}

