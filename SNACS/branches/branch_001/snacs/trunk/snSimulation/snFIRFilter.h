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
 * \file snFIRFilter.h
 *
 * \author Frank Schubert
 */

#ifndef SNFIRFILTER_H
#define SNFIRFILTER_H

#include <vector>
#include <iostream>

#include <itpp/itbase.h>
#include <itpp/signal/filter.h>

#include <QThread>

/**
 * \brief snFIRFilter implements a FIR filter that safes its own state
 * for processing of consecutive data blocks.
 *
 * The coefficients can be altered after every filter step without erasing the filter
 * state. The routine uses intrinsic SSE instruction if USE_SSE is defined.
 */
class snFIRFilter: public QThread {
public:
	//	snFIRFilter(double *pdataIn, double *pdataOut, int pdata_len, double *pcoeff, int pfilter_order);
	snFIRFilter(itpp::vec *pdataIn, itpp::vec *pdataOut, int pdata_len,
			itpp::vec *pcoeff, int pfilter_order);
	void run();

	void set_state(std::vector<double> s) {
		for (int i = 0; i < filter_order; i++)
			state[i] = s[i];
	}

	~snFIRFilter();
private:
	itpp::vec state;
	itpp::vec state_out;
	itpp::vec *dataIn;
	itpp::vec *dataOut;
	int n;
	itpp::vec *coeff;
	int filter_order;
};

#endif
