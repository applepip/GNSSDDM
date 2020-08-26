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
 * \file snFIRFilter.cpp
 *
 * \author Frank Schubert
 */

#include "snFIRFilter.h"

/**
 * \brief implements a FIR filter
 * @param pdataIn pointer to input array
 * @param pdataOut pointer to output array
 * @param pn number of input samples
 * @param pcoeff pointer to coefficients array
 * @param pm number of coefficients
 */
snFIRFilter::snFIRFilter(itpp::vec *pdataIn, itpp::vec *pdataOut,
		int pdata_len, itpp::vec *pcoeff, int pfilter_order) :
	QThread(), dataIn(pdataIn), dataOut(pdataOut), n(pdata_len), coeff(pcoeff),
			filter_order(pfilter_order) {

	//std::cout << "\n setting up new FIR filter. n = " << n << ", m = " << m;

	state.set_length(filter_order+1);
	state.zeros();

	state_out.set_length(state.size());
}

snFIRFilter::~snFIRFilter() {

}

void snFIRFilter::run() {

	*dataOut = itpp::filter(*coeff, 1, *dataIn, state, state_out);;

	state = state_out;
}
