/*
 * snModulationBPSK.h
 *
 *  Created on: Mar 10, 2010
 *      Author: frank
 */

#ifndef SNMODULATIONBPSK_H_
#define SNMODULATIONBPSK_H_

#include "snModulation.h"

class snModulationBPSK: public snModulation {
public:
	snModulationBPSK(snGNSSCode *_code, snGNSSData *_data, double _smpl_freq, double _n);
	virtual ~snModulationBPSK();

	double get_value_for_absolute_sample_number(Samples_Type v);
	//std::vector<double> get_one_code_block_sampled();
	double get_discriminator_gain() {
		return 1.0;
	}

private:
	double n;
};

#endif /* SNMODULATIONBPSK_H_ */
