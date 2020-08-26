/*
 * snModulationCBOC.cpp
 *
 *  Created on: Mar 17, 2010
 *      Author: frank
 */
#include <iostream>

#include "snModulationCBOC.h"

snModulationCBOC::snModulationCBOC(snGNSSCode *_code, snGNSSData *_data, double _smpl_freq, double _n1,
		double _n2, double _power_ratio) :
	snModulation(_code, _data, _smpl_freq), n1(_n1), n2(_n2), power_ratio(_power_ratio) {

	f_sc1 = n1 * code->get_code_freq_cps();
	f_sc2 = n2 * code->get_code_freq_cps();

//	std::cout << "CBOC power_ratio: " << power_ratio << "\n";
}

snModulationCBOC::~snModulationCBOC() {
	// TODO Auto-generated destructor stub
}

double snModulationCBOC::get_value_for_absolute_sample_number(Samples_Type v) {
	const double vmod = fmod(v, get_samples_per_code());
	const int code_index = floor(vmod / get_samples_per_code() * code->get_number_of_chips_per_code());
	const double code_chip = code->get_chip(code_index);

	const double subchip1 = sgn(sin(2.0 * M_PI * f_sc1 * v / smpl_freq));
	const double subchip2 = sgn(sin(2.0 * M_PI * f_sc2 * v / smpl_freq));
	//std::cout << "v: " << v << ", code_index: " << code_index << ", subchip: " << subchip << "\n";
	return (sqrt(power_ratio) * subchip1 + sqrt(1.0 - power_ratio) * subchip2) * code_chip
			* data->get_value_for_time(static_cast<double> (v) / smpl_freq);
}
/*
 std::vector<double> snModulationCBOC::get_one_code_block_sampled() {
 // TODO

 const double samples_per_subchip = smpl_freq / f_sc1 / 2.0;
 //const double samples_per_codechip = smpl_freq / f_c;

 Samples_Type acSmpls = code->get_number_of_chips_per_code()
 / code->get_code_freq_cps() * smpl_freq;
 std::vector<double> blk(acSmpls);

 for (Samples_Type i = 0; i < acSmpls; i++) {
 double code_chip = code->get_chip((int) floor(static_cast<double> (i)
 / static_cast<double> (acSmpls)
 * (code->get_number_of_chips_per_code() - 1)));

 // subchips as 0, 1, 0, 1:
 double subchip = fmod(floor(static_cast<double> (i)
 / samples_per_subchip), 2);
 // subchips as -1, 1, -1, 1:
 subchip = subchip * 2 - 1; // 0*2-1 = -1.  1*2-1 = 1.

 blk.at(i) = subchip * code_chip;
 }
 return blk;
 }

 */
