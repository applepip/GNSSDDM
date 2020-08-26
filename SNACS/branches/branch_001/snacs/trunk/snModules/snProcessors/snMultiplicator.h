/*
 * snProcessorMultiplicator.h
 *
 *  Created on: May 11, 2011
 *      Author: frank
 */

#ifndef SNPROCESSORMULTIPLICATOR_H_
#define SNPROCESSORMULTIPLICATOR_H_

#include "../snWidget/snWidget.h"
#include "../snSimulation/snCreatingModule.h"

class snMultiplicator: public snCreatingModule {
public:
	snMultiplicator(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *_snw, ConcBuf3* _circbuf_in);
	virtual ~snMultiplicator();

	void run();

private:

	double start_sec;
	double mult_data_freq;
	size_t nof_mult_data;
	itpp::cvec *mult_data;

	snWidget *snw;
	snCurveComplex *mult_curve;
	snCurveReal *mult_curve_dB, *mult_curve_angl;
	snCurveComplex *out_curve;

	ConcBuf3 *circbuf_in;
};

#endif /* SNPROCESSORMULTIPLICATOR_H_ */
