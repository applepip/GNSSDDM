/*
 * snModulationBOC.h
 *
 *  Created on: Mar 10, 2010
 *      Author: frank
 */

#ifndef SNMODULATIONBOC_H_
#define SNMODULATIONBOC_H_

#include "snModulation.h"

class snModulationBOC: public snModulation {
public:
	enum boc_phasing_type {
		SIN, COS
	};

	snModulationBOC(snGNSSCode *_code, snGNSSData *_data, double _smpl_freq, double _n, double _m,
			boc_phasing_type _boc_phasing);
	virtual ~snModulationBOC();

	double get_value_for_absolute_sample_number(Samples_Type v);
	double get_discriminator_gain() {
		return (2.0 * (4 * n / m - 1));
	}
private:
	double f_sc; ///< subcarrier frequency
	//double f_c; ///< code frequency
	double n, m;
	boc_phasing_type boc_phasing;
};

#endif /* SNMODULATIONBOC_H_ */
