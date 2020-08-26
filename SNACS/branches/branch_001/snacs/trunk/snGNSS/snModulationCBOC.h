/*
 * snModulationCBOC.h
 *
 *  Created on: Mar 17, 2010
 *      Author: frank
 */

#ifndef SNMODULATIONCBOC_H_
#define SNMODULATIONCBOC_H_

#include "snModulation.h"

class snModulationCBOC: public snModulation {
public:
	snModulationCBOC(snGNSSCode *_code, snGNSSData *_data, double _smpl_freq, double _n1,
			double _n2, double _power_ratio);
	virtual ~snModulationCBOC();

	double get_value_for_absolute_sample_number(Samples_Type v);
	//std::vector<double> get_one_code_block_sampled();
	double get_discriminator_gain() {
//		return 1.0 / (2.0 * (4 * 1 / 1 - 1)); // not right!
		return 6.0;
	}

private:
	double f_sc1, f_sc2; ///< subcarrier frequencies
	double n1, n2;
	double power_ratio;
};

#endif /* SNMODULATIONCBOC_H_ */
