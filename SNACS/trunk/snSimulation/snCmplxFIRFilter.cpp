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

	S1 = new double[n];
	S2 = new double[n];
	S3 = new double[n];
	S4 = new double[n];

	rX1 = new double[n];
	iX2 = new double[n];
	rX3 = new double[n];
	iX4 = new double[n];

	rH1 = new double[n];
	iH2 = new double[n];
	iH3 = new double[n];
	rH4 = new double[n];

	Filter1 = new snFIRFilter(rX1, S1, n, rH1, m);
	Filter2 = new snFIRFilter(iX2, S2, n, iH2, m);
	Filter3 = new snFIRFilter(rX3, S3, n, iH3, m);
	Filter4 = new snFIRFilter(iX4, S4, n, rH4, m);
}

snCmplxFIRFilter::~snCmplxFIRFilter() {
	delete[] S1;
	delete[] S2;
	delete[] S3;
	delete[] S4;
	delete[] rX1;
	delete[] iX2;
	delete[] rX3;
	delete[] iX4;
	delete[] rH1;
	delete[] iH2;
	delete[] iH3;
	delete[] rH4;
	delete Filter1;
	delete Filter2;
	delete Filter3;
	delete Filter4;

}

void snCmplxFIRFilter::filter(std::complex<double> *X, std::complex<double> *Y,
		const std::vector<std::complex<double> > H) {

	for (int i = 0; i < m; i++) {
		rH1[i] = H[i].real();
		iH2[i] = H[i].imag();
		rH4[i] = H[i].real();
		iH3[i] = H[i].imag();

	}

	for (int i = 0; i < n; i++) {
		rX1[i] = X[i].real();
		iX2[i] = X[i].imag();
		rX3[i] = X[i].real();
		iX4[i] = X[i].imag();
	}

	Filter1->start();
	Filter2->start();
	Filter3->start();
	Filter4->start();

	Filter1->wait();
	Filter2->wait();
	Filter3->wait();
	Filter4->wait();

	for (int i = 0; i < n; i++) {
		Y[i].real() = S1[i] - S2[i];
		Y[i].imag() = S3[i] + S4[i];
	}
}

