/*
 * snModulationBOC.cpp
 *
 *  Created on: Mar 10, 2010
 *      Author: frank
 */

#include <iostream>

#include "snModulationBOC.h"

snModulationBOC::snModulationBOC(snGNSSCode *_code, snGNSSData *_data,
		double _smpl_freq, double _n, double _m, boc_phasing_type _boc_phasing) :
	snModulation(_code, _data, _smpl_freq), n(_n), m(_m), boc_phasing(
			_boc_phasing) {

	f_sc = n * 1.023e6;
	code->set_freq_cps(m * 1.023e6);
}

snModulationBOC::~snModulationBOC() {
	// TODO Auto-generated destructor stub
}

double snModulationBOC::get_value_for_absolute_sample_number(Samples_Type v) {
	const double v_double = static_cast<double> (v);
	const double samples_per_code = get_samples_per_code();

	const double vmod = fmod(v_double, samples_per_code); // unterschied ab 16000?
	//std::cout << " v: " << v << "vmod: " << vmod << ", ";

	/*
	const int code_index = floor(vmod / samples_per_code
			* static_cast<double> (code->get_number_of_chips_per_code()));
*/

	const int code_index = floor(vmod * code->get_code_freq_cps() / smpl_freq);

	const double code_chip = code->get_chip(code_index);

	double subchip = 0.0;
	//double phase = 2.0 * M_PI * f_sc * v_double / smpl_freq;
	// hier vmod?
	double phase = 2.0 * M_PI * f_sc * vmod / smpl_freq;
	phase = fmod(phase, 2.0 * M_PI);
	switch (boc_phasing) {
	case SIN:
		subchip = sgn(sin(phase));
		break;
	case COS:
		subchip = sgn(cos(phase));
		break;
	}
	//std::cout << "v: " << v << ", code_index: " << code_index << ", subchip: " << subchip << "\n";
	return code_chip * subchip;
	//	return  subchip;
}
