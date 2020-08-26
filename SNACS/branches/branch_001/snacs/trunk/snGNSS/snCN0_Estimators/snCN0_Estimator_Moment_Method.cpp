/*
 * snCN0_Estimator_Moment_Method.cpp
 *
 *  Created on: Apr 14, 2011
 *      Author: frank
 */

#include "snCN0_Estimator_Moment_Method.h"

/*
 * implements a SNR estimator based on the moment method, see InsideGNSS 01,02/2010
 */
snCN0_Estimator_Moment_Method::snCN0_Estimator_Moment_Method(size_t _nof_averages) :
	nof_averages(_nof_averages) {
	// TODO Auto-generated constructor stub

}

snCN0_Estimator_Moment_Method::~snCN0_Estimator_Moment_Method() {
	// TODO Auto-generated destructor stub
}

double snCN0_Estimator_Moment_Method::get_estimation(const itpp::cvec & correlation_results_prompt,
		size_t current_interval) {

	if (current_interval > nof_averages) {
		itpp::cvec samples = correlation_results_prompt.mid(current_interval - nof_averages, nof_averages);
		//std::cout << "\nsamples.size(): " << samples.size() << "\n";
		const double M2 = 1.0 / nof_averages * itpp::sum(itpp::pow(itpp::abs(samples), 2.0));
		const double M4 = 1.0 / nof_averages * itpp::sum(itpp::pow(itpp::abs(samples), 4.0));
		const double Pd = sqrt(2.0 * M2 * M2 - M4);
		const double Pn = M2 - Pd;
		return Pd / Pn;
	}
	return 1.0;
}
