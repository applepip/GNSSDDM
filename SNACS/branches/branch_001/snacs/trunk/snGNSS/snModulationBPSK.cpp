/*
 * snModulationBPSK.cpp
 *
 *  Created on: Mar 10, 2010
 *      Author: frank
 */

#include <iostream>

#include "snModulationBPSK.h"

snModulationBPSK::snModulationBPSK(snGNSSCode *_code, snGNSSData *_data, double _smpl_freq,
		double _n) :
	snModulation(_code, _data, _smpl_freq), n(_n) {

//	std::cout << "\nBPSK setup. get_samples_per_code(): " << get_samples_per_code()
//			<< ", get_samples_per_chip(): " << get_samples_per_chip() << "\n";

	code->set_freq_cps(n * 1.023e6);

}

snModulationBPSK::~snModulationBPSK() {
	// TODO Auto-generated destructor stub
}

double snModulationBPSK::get_value_for_absolute_sample_number(Samples_Type v) {
	const double vmod = fmod(static_cast<double> (v), get_samples_per_code());
	//const int index = floor(vmod / get_samples_per_code()
	//		* static_cast<double> (code->get_number_of_chips_per_code()));
	const int index = floor(vmod * code->get_code_freq_cps() / smpl_freq);
	//std::cout << "BPSK: v: " << v << ", index: " << index << "\n";
	const double out = code->get_chip(index) * data->get_value_for_time(static_cast<double> (v)
			/ smpl_freq);
	return out;
}
