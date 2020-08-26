/*
 * snParallelCorrelation.h
 *
 *  Created on: Mar 12, 2010
 *      Author: frank
 */

#ifndef SNPARALLELCORRELATION_H_
#define SNPARALLELCORRELATION_H_

#include <vector>

#include <QThread>

class snParallelCorrelation: public QThread {
public:
	//	snParallelCorrelation(const std::vector<double> &_s1, const std::vector<double> &_s2I,
	//			std::vector<double> &_s2Q, double *_I, double *_Q);
	snParallelCorrelation( std::vector<double> *_s1,  std::vector<double> *_s2I,
			std::vector<double> *_s2Q, double *_I, double *_Q);
	virtual ~snParallelCorrelation();

	void run();
private:
	std::vector<double> *s1;
	std::vector<double> *s2I;
	std::vector<double> *s2Q;
	double *I;
	double *Q;
};

#endif /* SNPARALLELCORRELATION_H_ */
