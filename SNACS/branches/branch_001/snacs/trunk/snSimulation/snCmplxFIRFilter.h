/*
 *  SNACS - The Satellite Navigation Radio Channel Simulator
 *
 *  Copyright (C) 2009  F. M. Schubert
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * \file snCmplxFIRFilter.h
 *
 * \author Frank Schubert
 */

#ifndef SNCMPLXFIRFILTER_H
#define SNCMPLXFIRFILTER_H

#include <iostream>
#include <complex>
#include <vector>

#include "snFIRFilter.h"

/**
 * \brief This class implements a multi-threaded complex FIR filter.
 *
 * It uses four instances of a snFIRFilter for the actual filtering.
 */
class snCmplxFIRFilter {
public:

	/**
	 * \brief FIR filter constructor
	 *
	 * \param <int pn> input data vector length
	 * \param <int pm> filter order
	 */
	snCmplxFIRFilter(int pn, int pm);

	/**
	 * \brief filters the data
	 *
	 * \param <std::complex<double> *X> pointer to input data
	 * \param <std::complex<double> *Y> pointer to output data
	 * \param <std::vector<std::complex<double> > H> coefficients
	 */
	void filter(const itpp::cvec &X, itpp::cvec &Y, const itpp::cvec& H);
	~snCmplxFIRFilter();

private:

	int n, m;
	snFIRFilter *Filter1, *Filter2, *Filter3, *Filter4;
	itpp::vec S1, S2, S3, S4, rX1, iX2, rX3, iX4, rH1, iH2, iH3, rH4;
	itpp::cvec state;
	itpp::cvec state_out;

};

#endif
