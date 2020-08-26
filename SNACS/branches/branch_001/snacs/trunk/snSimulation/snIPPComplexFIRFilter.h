/*
 * snIPPComplexFIRFilter.h
 *
 *  Created on: Aug 20, 2010
 *      Author: fschubert
 */

#ifdef HAVE_IPP

#include "snGlobal.h"

#include "ipps.h"

#ifndef SNIPPCOMPLEXFIRFILTER_H_
#define SNIPPCOMPLEXFIRFILTER_H_

class snIPPRealFIRFilter {
public:
	snIPPRealFIRFilter(int _datalen, int _numtaps);
	virtual ~snIPPRealFIRFilter();

	void set_coeffs(itpp::vec& H);
	void filter(itpp::vec &X, itpp::vec &Y);

private:
	int datalen, numtaps;
	IppStatus status;

	IppsFIRState_64f *state;

	Ipp64f* x;
	Ipp64f* y;
	Ipp64f* taps;
};


class snIPPComplexFIRFilter {
public:
	snIPPComplexFIRFilter(int _datalen, int _numtaps);
	virtual ~snIPPComplexFIRFilter();

	void set_coeffs(itpp::cvec& H);
	void filter(itpp::cvec &X, itpp::cvec &Y);

private:
	int datalen, numtaps;
	IppStatus status;

	IppsFIRState_64fc *state;

	Ipp64fc* x;
	Ipp64fc* y;
	Ipp64fc* taps;
};

#endif /* SNIPPCOMPLEXFIRFILTER_H_ */

#endif
