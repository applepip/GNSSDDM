/*
 * snParallelCorrelation.cpp
 *
 *  Created on: Mar 12, 2010
 *      Author: frank
 */

#include "snParallelCorrelation.h"

#include <cstdlib>
#include <iostream>

//snParallelCorrelation::snParallelCorrelation(const std::vector<double> &_s1, const std::vector<double> &_s2I,
//		std::vector<double> &_s2Q, double *_I, double *_Q) :
snParallelCorrelation::snParallelCorrelation(std::vector<double> *_s1, std::vector<double> *_s2I,
		std::vector<double> *_s2Q, double *_I, double *_Q) :
	QThread(), s1(_s1), s2I(_s2I), s2Q(_s2Q), I(_I), Q(_Q) {

}

snParallelCorrelation::~snParallelCorrelation() {
	// TODO Auto-generated destructor stub
}

void snParallelCorrelation::run() {

	if ((s1->size() != s2I->size()) || (s2I->size() != s2Q->size())) {
		std::cerr << "snParallelCorrelation: input vectors have different sizes. abort.";
		std::exit(1);
	}

	double Iout = 0.0;
	double Qout = 0.0;

	size_t k = s1->size();

	for (size_t i = 0; i < k; i++) {
		Iout += (*s1)[i] * (*s2I)[i];
		Qout += (*s1)[i] * (*s2Q)[i];
	}

	*I = Iout;
	*Q = Qout;
}
