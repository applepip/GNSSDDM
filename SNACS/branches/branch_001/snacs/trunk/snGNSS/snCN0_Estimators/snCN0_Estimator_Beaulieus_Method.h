/*
 * snCN0_Estimator_Beaulieus_Method.h
 *
 *  Created on: Apr 14, 2011
 *      Author: frank
 */

#ifndef SNCN0_ESTIMATOR_BEAULIEUS_METHOD_H_
#define SNCN0_ESTIMATOR_BEAULIEUS_METHOD_H_

#include <itpp/itbase.h>

class snCN0_Estimator_Beaulieus_Method {
public:
	snCN0_Estimator_Beaulieus_Method(size_t _nof_averages);
	virtual ~snCN0_Estimator_Beaulieus_Method();

	double get_estimation(const itpp::cvec & correlation_results_prompt, size_t current_interval);

private:
	double Pnv(const itpp::vec & r, size_t v);
	double Pdv(const itpp::vec & r, size_t v);

	size_t nof_averages;

};

#endif /* SNCN0_ESTIMATOR_BEAULIEUS_METHOD_H_ */
