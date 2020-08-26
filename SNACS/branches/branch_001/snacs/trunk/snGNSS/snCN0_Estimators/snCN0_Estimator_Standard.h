/*
 * snCN0_Estimator_Standard.h
 *
 *  Created on: Sep 8, 2010
 *      Author: fschubert
 */

#ifndef SNCN0_ESTIMATOR_STANDARD_H_
#define SNCN0_ESTIMATOR_STANDARD_H_

#include <vector>

/*
 * This class implements the standard C/N0 estimator according to
 * v. Dierendonck's chapter in the blue books.
 */

#include <itpp/itbase.h>

class snCN0_Estimator_Standard {
public:
	snCN0_Estimator_Standard(double _corr_len, size_t _nof_averages);
	virtual ~snCN0_Estimator_Standard();

	double get_estimation(const itpp::cvec & correlation_results_prompt, size_t current_interval);

	double WBP(const itpp::vec & I, const itpp::vec & Q, size_t k);
	double NBP(const itpp::vec & I, const itpp::vec & Q, size_t k);

private:
	double corr_len;
	size_t nof_averages;

	size_t M;
};

#endif /* SNCN0_ESTIMATOR_STANDARD_H_ */
