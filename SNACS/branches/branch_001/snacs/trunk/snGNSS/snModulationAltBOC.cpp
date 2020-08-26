/*
 * snModulationAltBOC.cpp
 *
 *  Created on: 18.03.2010
 *      Author: schu_fr
 */

#include "snModulationAltBOC.h"

#include <cstdlib>
#include <iostream>
#include <cmath>

snModulationAltBOC::snModulationAltBOC(snGNSSCode *_code_E5a_I, snGNSSCode *_code_E5a_Q,
		snGNSSCode *_code_E5b_I, snGNSSCode *_code_E5b_Q, snGNSSData *_data, double _smpl_freq, double _n,
		double _m, path_type _pt) :
	snModulation(_code_E5a_I, _data, _smpl_freq), code_E5a_I(_code_E5a_I), code_E5a_Q(_code_E5a_Q),
			code_E5b_I(_code_E5b_I), code_E5b_Q(_code_E5b_Q), n(_n), m(_m), pt(_pt) {

	f_sc = n * 1.023e6; // Hz
//	std::cout << "AltBOC subcarrier freq: " << f_sc << ", chip rate: " << m * 1.023e6 << "\n";
	t_sc = 1.0 / f_sc;

	code_E5a_I->set_freq_cps(m * 1.023e6);
	code_E5a_Q->set_freq_cps(m * 1.023e6);
	code_E5b_I->set_freq_cps(m * 1.023e6);
	code_E5b_Q->set_freq_cps(m * 1.023e6);

	code->set_freq_cps(m * 1.023e6); // this one as code to be tracked
}

snModulationAltBOC::~snModulationAltBOC() {
	// TODO Auto-generated destructor stub
}

double snModulationAltBOC::get_value_for_absolute_sample_number(Samples_Type v) {
	// TODO add data
	const double vmod = fmod(v, get_samples_per_code());
	const int code_index = floor(vmod / get_samples_per_code() * code->get_number_of_chips_per_code());

	const double e1 = code_E5a_I->get_chip(code_index);
	const double e2 = code_E5a_Q->get_chip(code_index);
	const double e3 = code_E5b_I->get_chip(code_index);
	const double e4 = code_E5b_Q->get_chip(code_index);

	const double e1_d = e2 * e3 * e4;
	const double e2_d = e1 * e3 * e4;
	const double e3_d = e1 * e2 * e4;
	const double e4_d = e1 * e2 * e3;

	const double t = v / smpl_freq;
	const int phase_point = get_phase_point_0_to_7(t);
	const int phase_point_Ts4 = get_phase_point_0_to_7(t - t_sc / 4.0);

	const double subchip1 = get_sc_E5_S(t, phase_point);
	const double subchip1_Ts4 = get_sc_E5_S(t - t_sc / 4.0, phase_point_Ts4);
	const double subchip2 = get_sc_E5_P(t, phase_point);
	const double subchip2_Ts4 = get_sc_E5_P(t - t_sc / 4.0, phase_point_Ts4);
	//std::cout << "v: " << v << ", code_index: " << code_index << ", subchip: " << subchip << "\n";

	//	subchip2 = 0.0;
	//	subchip2_Ts4 = 0.0;
	double out = 0.0;
	switch (pt) {
	case INPHASE:
		out = (1.0 / (2.0 * sqrt(2.0))) * ((e1 + e3) * subchip1 + (e2 - e4) * subchip1_Ts4 + (e1_d + e3_d)
				* subchip2 + (e2_d - e4_d) * subchip2_Ts4);
		break;
	case QUADRATURE:
		out = (1.0 / (2.0 * sqrt(2.0))) * ((e2 + e4) * subchip1 + (e3 - e1) * subchip1_Ts4 + (e2_d + e4_d)
				* subchip2 + (e3_d - e1_d) * subchip2_Ts4);
		break;
	}
	return out;
}

int snModulationAltBOC::get_phase_point_0_to_7(double t) {
	/*	if (t < 0)
	 return 0.0;
	 else
	 */
	double out = floor(std::fmod(t, t_sc) / (t_sc / 8.0));
	if (out < 0)
		out = -out;
	return out;
	//	return floor((t % t_sc) / (t_sc / 8.0));
}

double snModulationAltBOC::get_sc_E5_S(double t, int phase_point) {
	double out;
	//std::cout << "phase point: " << get_phase_point_0_to_7(t) << "\n";
	//	const int phase_point = get_phase_point_0_to_7(t);
	switch (phase_point) {
	case 0:
		out = sqrt(2.0) + 1;
		break;
	case 1:
		out = 1.0;
		break;
	case 2:
		out = -1.0;
		break;
	case 3:
		out = -sqrt(2.0) - 1.0;
		break;
	case 4:
		out = -sqrt(2.0) - 1.0;
		break;
	case 5:
		out = -1.0;
		break;
	case 6:
		out = 1.0;
		break;
	case 7:
		out = sqrt(2.0) + 1;
		break;
	default:
		std::cout << "snModulationAltBOC::get_sc_E5_S: parameter (" << phase_point
				<< ") not btw 0 and 7, must not happen. abort." << "\n";
		std::abort();
		break;
	}
	//std::cout << "out: " << out << "\n";
	return out / 2.0;
}

double snModulationAltBOC::get_sc_E5_P(double t, int phase_point) {
	double out;
	//const int phase_point = get_phase_point_0_to_7(t);
	switch (phase_point) {
	case 0:
		out = -sqrt(2.0) + 1;
		break;
	case 1:
		out = 1.0;
		break;
	case 2:
		out = -1.0;
		break;
	case 3:
		out = sqrt(2.0) - 1.0;
		break;
	case 4:
		out = sqrt(2.0) - 1.0;
		break;
	case 5:
		out = -1.0;
		break;
	case 6:
		out = 1.0;
		break;
	case 7:
		out = -sqrt(2.0) + 1;
		break;
	default:
		std::cout << "snModulationAltBOC::get_sc_E5_S: parameter (" << phase_point
				<< ") not btw 0 and 7, must not happen. abort." << "\n";
		std::abort();
		break;
	}
	return out / 2.0;
}

