/*
 * snCN0_Estimator_Beaulieus_Method.cpp
 *
 *  Created on: Apr 14, 2011
 *      Author: frank
 */

#include "snCN0_Estimator_Beaulieus_Method.h"

snCN0_Estimator_Beaulieus_Method::snCN0_Estimator_Beaulieus_Method(size_t _nof_averages) :
	nof_averages(_nof_averages) {
	// TODO Auto-generated constructor stub

}

snCN0_Estimator_Beaulieus_Method::~snCN0_Estimator_Beaulieus_Method() {
	// TODO Auto-generated destructor stub
}

double snCN0_Estimator_Beaulieus_Method::Pnv(const itpp::vec & r, size_t v) {
	return pow(fabs(r(v)) - fabs(r(v - 1)), 2.0);
}

double snCN0_Estimator_Beaulieus_Method::Pdv(const itpp::vec & r, size_t v) {
	return .5 * (pow(r(v), 2.0) + pow(r(v - 1), 2.0));
}

double snCN0_Estimator_Beaulieus_Method::get_estimation(const itpp::cvec & correlation_results_prompt,
		size_t current_interval) {

	if (current_interval > nof_averages) {
		itpp::vec samples_r = itpp::real(correlation_results_prompt.mid(current_interval - nof_averages,
				nof_averages));

		double sum = 0.0;
		for (size_t k = 1; k < nof_averages; k++) { // start loop at 1, since k-1 is used in Pnv and Pdv!
			sum += Pnv(samples_r, k) / Pdv(samples_r, k);
		}
		return 1.0 / (1.0 / (nof_averages - 1) * sum); // since we started the loop above at 1, subtract 1 of nof_averages! (nof_averages - 1)
	}
	return 1.0;
}
