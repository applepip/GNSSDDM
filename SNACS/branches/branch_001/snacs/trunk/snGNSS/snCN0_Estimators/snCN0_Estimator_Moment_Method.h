/*
 * snCN0_Estimator_Moment_Method.h
 *
 *  Created on: Apr 14, 2011
 *      Author: frank
 */

#ifndef SNCN0_ESTIMATOR_MOMENT_METHOD_H_
#define SNCN0_ESTIMATOR_MOMENT_METHOD_H_

#include <itpp/itbase.h>

class snCN0_Estimator_Moment_Method {
public:
	snCN0_Estimator_Moment_Method(size_t _nof_averages);
	virtual ~snCN0_Estimator_Moment_Method();

	double get_estimation(const itpp::cvec & correlation_results_prompt, size_t current_interval);

private:
	size_t nof_averages;
};

#endif /* SNCN0_ESTIMATOR_MOMENT_METHOD_H_ */
