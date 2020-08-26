/*
 * snModulationAltBOC.h
 *
 *  Created on: 18.03.2010
 *      Author: schu_fr
 */

#ifndef SNMODULATIONALTBOC_H_
#define SNMODULATIONALTBOC_H_

#include "snModulation.h"

class snModulationAltBOC: public snModulation {
public:
	enum path_type {
		INPHASE, QUADRATURE
	};

	snModulationAltBOC(snGNSSCode *_code_E5a_I, snGNSSCode *_code_E5a_Q, snGNSSCode *_code_E5b_I,
			snGNSSCode *_code_E5b_Q, snGNSSData *_data, double _smpl_freq, double _n, double _m, path_type _pt);
	virtual ~snModulationAltBOC();

	double get_value_for_absolute_sample_number(Samples_Type v);
	double get_discriminator_gain() {
		// TODO
		return 1.0 / (2.0 * (4 * n / m - 1));
	}

private:

	int get_phase_point_0_to_7(double t);
	double get_sc_E5_S(double t, int phase_point);
	double get_sc_E5_P(double t, int phase_point);

	double f_sc, t_sc; // subcarrier
	//double f_c; // code

	snGNSSCode *code_E5a_I, *code_E5a_Q, *code_E5b_I, *code_E5b_Q;
	double n, m;
	path_type pt;

};

#endif /* SNMODULATIONALTBOC_H_ */
