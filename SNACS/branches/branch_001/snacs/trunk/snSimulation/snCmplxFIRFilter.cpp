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
 * \file snCmplxFIRFilter.cpp
 *
 * \author Frank Schubert
 */

#include "snFIRFilter.h"
#include "snCmplxFIRFilter.h"

snCmplxFIRFilter::snCmplxFIRFilter(int pn, int pm) :
	n(pn), m(pm) {

	state.set_size(m + 1);
	state.zeros();

	state_out.set_size(state.size());

	S1.set_size(n);
	S2.set_size(n);
	S3.set_size(n);
	S4.set_size(n);

	rX1.set_size(n);
	iX2.set_size(n);
	rX3.set_size(n);
	iX4.set_size(n);

	rH1.set_size(n);
	iH2.set_size(n);
	iH3.set_size(n);
	rH4.set_size(n);

	Filter1 = new snFIRFilter(&rX1, &S1, n, &rH1, m);
	Filter2 = new snFIRFilter(&iX2, &S2, n, &iH2, m);
	Filter3 = new snFIRFilter(&rX3, &S3, n, &iH3, m);
	Filter4 = new snFIRFilter(&iX4, &S4, n, &rH4, m);
}

snCmplxFIRFilter::~snCmplxFIRFilter() {
	delete Filter1;
	delete Filter2;
	delete Filter3;
	delete Filter4;
}

void snCmplxFIRFilter::filter(const itpp::cvec &X, itpp::cvec &Y,
		const itpp::cvec & H) {

	//cvec itpp::filter (const cvec &b, const int one, const cvec &input, const cvec &state_in, cvec &state_out)

	// serial version:
	/*
	 Y = itpp::filter(H, 1, X, state, state_out);
	 state = state_out;
	 */

	// parallel version:
	const itpp::vec H_real = itpp::real(H);
	const itpp::vec X_real = itpp::real(X);

	rH1 = H_real;
	rX1 = X_real;
	Filter1->start();

	const itpp::vec X_imag = itpp::imag(X);
	const itpp::vec H_imag = itpp::imag(H);

	iH2 = H_imag;
	iX2 = X_imag;
	Filter2->start();

	iH3 = H_imag;
	rX3 = X_real;
	Filter3->start();

	rH4 = H_real;
	iX4 = X_imag;
	Filter4->start();

	Filter1->wait();
	Filter2->wait();
	Filter3->wait();
	Filter4->wait();

	Y = to_cvec(S1 - S2, S3 + S4);

}

