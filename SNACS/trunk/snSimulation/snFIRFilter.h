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

#include <QThread>

#ifdef USE_SSE
#include "emmintrin.h"
#endif

/**
 * \brief snFIRFilter implements a FIR filter that safes its own state
 * for processing of consecutive data blocks.
 *
 * The coefficients can be altered after every filter step without erasing the filter
 * state. The routine uses intrinsic SSE instruction if USE_SSE is defined.
 */
class snFIRFilter: public QThread {
public:
			snFIRFilter(double *pdataIn, double *pdataOut, int pdata_len,  double *pcoeff,
					int pfilter_order);
	void run();

	void set_state(std::vector<double> s) {
		for (int i = 0; i < filter_order; i++)
			state[i] = s[i];
	}

	std::vector<double> get_state() {
		std::vector<double> out(filter_order);
		for (int i = 0; i < filter_order; i++)
			out[i] = state[i];
		return out;
	}
/*
	void set_datalen(int new_n) {
		n = new_n;
	}
*/
	~snFIRFilter();
private:
	double *state;
	double *dataIn;
	double *dataOut;
	int n;
	double *coeff __attribute__ ( ( aligned ( 16 ) ) );
	int filter_order;

	double *x __attribute__ ( ( aligned ( 16 ) ) );
	double *y __attribute__ ( ( aligned ( 16 ) ) );
	double *c __attribute__ ( ( aligned ( 16 ) ) );
	double *x2 __attribute__ ( ( aligned ( 16 ) ) );
	double zerot[2] __attribute__ ( ( aligned ( 16 ) ) );

#ifdef USE_SSE
	__m128d *rx, *ry, *rc, *rx2;
#endif

	double res[2] __attribute__ ( ( aligned ( 16 ) ) );
};

#endif
