/*
 * snParallelBuildCode.cpp
 *
 *  Created on: Mar 12, 2010
 *      Author: frank
 */

#include "snParallelBuildCode.h"

snParallelBuildCode::snParallelBuildCode(std::vector<double> *_code,
		int *_code_len, double *_code_phase_remainder, double _shift,
		snModulation *_modulation) :
	QThread(), code(_code), code_len(_code_len), code_phase_remainder(
			_code_phase_remainder), shift(_shift), modulation(_modulation) {

}

snParallelBuildCode::~snParallelBuildCode() {
	// TODO Auto-generated destructor stub
}

void snParallelBuildCode::run() {
	int blksize = code->size();
	for (int i = 0; i < blksize; i++) {
		/*
		 //old:
		 int ind = static_cast<int> (*code_phase_remainder + i * (*code_phase_step) + shift);
		 if (ind < 0) {
		 ind += *code_len;
		 }
		 if (ind >= *code_len) {
		 ind -= *code_len;
		 }

		 code->at(i) = code_corr->at(ind);
		 */
		/*
		 // new:
		 // build early code:
		 double ind_d = (*code_phase_remainder + static_cast<double> (i)
		 * (*code_phase_step) + shift);
		 if (ind_d < 0) {
		 ind_d += static_cast<double> (*code_len);
		 }
		 if (ind_d >= *code_len) {
		 //std::cout << "ind_d >= code_len : " << ind_d << ", code_len: " << code_len << "\n";
		 ind_d -= static_cast<double> (*code_len);
		 }

		 unsigned int ind_d_in_samples = round(ind_d
		 * modulation->get_samples_per_chip());
		 code->at(i) = modulation->get_value_for_absolute_sample_number(
		 ind_d_in_samples);
		 */

		// new2:
		// build code:
		const double samples_per_chip = modulation->get_samples_per_chip();
		const double samples_per_code = modulation->get_samples_per_code();
		double ind_d = *code_phase_remainder * samples_per_chip
				+ static_cast<double> (i) + shift * samples_per_chip;
		if (ind_d < 0) {
			ind_d += samples_per_code;
		}
		code->at(i) = modulation->get_value_for_absolute_sample_number(round(
				ind_d));

	}
}
