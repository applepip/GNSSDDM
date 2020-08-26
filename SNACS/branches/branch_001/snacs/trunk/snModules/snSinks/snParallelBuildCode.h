/*
 * snParallelBuildCode.h
 *
 *  Created on: Mar 12, 2010
 *      Author: frank
 */

#ifndef SNPARALLELBUILDCODE_H_
#define SNPARALLELBUILDCODE_H_

#include <vector>

#include <QThread>

#include "../snGNSS/snModulation.h"

class snParallelBuildCode: public QThread {
public:
	snParallelBuildCode(std::vector<double> *_code, int *_code_len,
			double *_code_phase_remainder, double _shift, snModulation *_modulation);
	virtual ~snParallelBuildCode();

	void run();

private:
	std::vector<double> *code;
	int *code_len;
	double *code_phase_remainder;
	double shift;
	snModulation *modulation;

	std::vector<double> *code_corr;
};

#endif /* SNPARALLELBUILDCODE_H_ */
