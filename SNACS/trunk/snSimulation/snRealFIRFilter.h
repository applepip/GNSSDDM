/*
 * snRealFIRFilter.h
 *
 *  Created on: Mar 28, 2010
 *      Author: frank
 */

#ifndef SNREALFIRFILTER_H_
#define SNREALFIRFILTER_H_

class snRealFIRFilter {
public:
	snRealFIRFilter();
	virtual ~snRealFIRFilter(int pn, int pm);

	void filter(double *X, double *Y, const std::vector<double> H);
};

#endif /* SNREALFIRFILTER_H_ */
