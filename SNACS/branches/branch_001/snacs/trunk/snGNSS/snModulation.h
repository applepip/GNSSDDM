/*
 * snModulation.h
 *
 *  Created on: Mar 10, 2010
 *      Author: frank
 */

#ifndef SNMODULATION_H_
#define SNMODULATION_H_

#include "../snSimulation/snGlobal.h"
#include "snGNSSCode.h"
#include "snGNSSData.h"

class snModulation {
public:
	snModulation(snGNSSCode *_code, snGNSSData *_data, double _smpl_freq);
	virtual ~snModulation();

	double get_samples_per_code() {
		//		return code->get_code_length_time() * smpl_freq;
		return smpl_freq
				* static_cast<double> (code->get_number_of_chips_per_code())
				/ code->get_code_freq_cps();
	}

	double get_samples_per_chip() {
		//return code->get_samples_per_chip(smpl_freq);
		return smpl_freq / code->get_code_freq_cps();
	}

	snGNSSCode* get_code() {
		return code;
	}

	virtual double get_value_for_absolute_sample_number(Samples_Type v) = 0;

	virtual itpp::vec get_one_code_block_sampled() {
		//	Samples_Type acSmpls = modulation->get_code_length() * SmplFreq;
		Samples_Type acSmpls = get_samples_per_code();
		itpp::vec blk(acSmpls);

		for (unsigned int i = 0; i < acSmpls; i++) {
			blk[i] = get_value_for_absolute_sample_number(i);
		}
		return blk;
	}

	virtual double get_discriminator_gain() = 0;

protected:
	double sgn(double v) {
		if (v <= 0)
			return -1.0;
		else
			return 1.0;
	}

	snGNSSCode *code;
	snGNSSData *data;
	double smpl_freq;
};

#endif /* SNMODULATION_H_ */
