/*
 * snCN0_Estimator_Standard.cpp
 *
 *  Created on: Apr 19, 2011
 *      Author: frank
 */

#include "snCN0_Estimator_Standard.h"

snCN0_Estimator_Standard::snCN0_Estimator_Standard(double _corr_len, size_t _nof_averages) :
	corr_len(_corr_len), nof_averages(_nof_averages) {

	M = 20;
}

snCN0_Estimator_Standard::~snCN0_Estimator_Standard() {
	// TODO Auto-generated destructor stub
}

double snCN0_Estimator_Standard::get_estimation(const itpp::cvec & correlation_results_prompt,
		size_t current_interval) {
	if (current_interval > (nof_averages + M + 1)) {

		const itpp::cvec corr_results = correlation_results_prompt.mid(current_interval - (nof_averages + M
				+ 1), nof_averages + M + 1);

		const itpp::vec I = itpp::real(corr_results);
		const itpp::vec Q = itpp::imag(corr_results);

		double muNP_sum = 0;
		for (size_t k = 0; k < nof_averages; k++) {
			const size_t index = corr_results.size() - 1 - k;
			muNP_sum += NBP(I, Q, index) / WBP(I, Q, index);
		}

		const double muNP = (1.0 / static_cast<double> (nof_averages)) * muNP_sum;

		return (1.0 / corr_len) * (muNP - 1.0) / (static_cast<double> (M) - muNP);
	} else
		return 1.0;
}

double snCN0_Estimator_Standard::WBP(const itpp::vec & I, const itpp::vec & Q, size_t k) {
	double sum = 0;
	for (size_t i = 0; i < M; i++) {
		sum += pow(I[k - i], 2.0) + pow(Q[k - i], 2.0);
	}
	return sum;
}

double snCN0_Estimator_Standard::NBP(const itpp::vec & I, const itpp::vec & Q, size_t k) {
	double sum1 = 0;
	for (size_t i = 0; i < M; i++) {
		sum1 += I[k - i];
	}
	double sum2 = 0;
	for (size_t i = 0; i < M; i++) {
		sum2 += Q[k - i];
	}
	return pow(sum1, 2.0) + pow(sum2, 2.0);
}

