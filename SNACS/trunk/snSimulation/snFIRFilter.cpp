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
snFIRFilter::snFIRFilter(double *pdataIn, double *pdataOut, int pdata_len,
		 double *pcoeff, int pfilter_order) :
	QThread(), dataIn(pdataIn), dataOut(pdataOut), n(pdata_len), coeff(pcoeff),
			filter_order(pfilter_order) {

	//std::cout << "\n setting up new FIR filter. n = " << n << ", m = " << m;

	state = new double[filter_order];

	for (int i = 0; i < filter_order; i++)
		state[i] = 0;

	// build buffer with [ state, dataIn ]:
	x = new double[n + filter_order + filter_order];
	y = new double[n + filter_order + filter_order];
	c = new double[filter_order];
	x2 = new double[2];

	for (int i = 0; i < (n + filter_order + filter_order); i++) {
		x[i] = 0;
		y[i] = 0;
	}

#ifdef USE_SSE
	rx = ( __m128d* ) x;
	ry = ( __m128d* ) y;
	rc = ( __m128d* ) c;
	rx2 = ( __m128d* ) x2;
#endif

	zerot[0] = 0.0;
	zerot[1] = 0.0;

}

snFIRFilter::~snFIRFilter() {
	delete[] state;
	delete[] x;
	delete[] y;
	delete[] c;
	delete[] x2;
}

void snFIRFilter::run() {
#ifdef USE_SSE

	for (int i = 0; i < filter_order; i++ )
	{
		x[i] = state[i];
		c[i] = coeff[filter_order-1-i]; // turn around coeffs
		//		c[i] = coeff[i];
	}

	for (int i = filter_order; i < filter_order+n; i++ )
	{
		x[i] = dataIn[i-filter_order];
	}

	//_mm_empty();
	__m128d t; // t will accumulate all multiplication results.
//	__m128d r1, r2;

	for (int i = 0; i < filter_order+n; i++ )
	{
		t = _mm_setzero_pd();
		//y[i] = 0;
		for (int o = 0; o < filter_order; o += 2 )
		//for ( o = 0; o < m; o++)
		{
			/*
			 //			r1 = _mm_set_pd  ( c[o],  c[o+1] );
			 r1 = _mm_loadu_pd(&c[o]);
			 //			r2 = _mm_set_pd   ( x[i+o+1], x[i+o+2] ); // use this for turned around coeffs
			 r2 = _mm_loadu_pd (&x[i+o+1]); // use this for turned around coeffs
			 //r2 = _mm_set_pd(x[i+m-o], x[i+m-o-1]); // coeffs are not turned
			 t = _mm_add_pd ( _mm_mul_pd ( r1, r2 ), t );
			 */

			t = _mm_add_pd ( _mm_mul_pd ( _mm_loadu_pd(&c[o]), _mm_loadu_pd (&x[i+o+1]) ), t );

		}
		_mm_storeu_pd ( ( double * ) res, t );
		y[i] = res[0] + res[1];
	}
	//_mm_empty();
	for (int i = 0; i < n; i++ )
	{
		dataOut[i] = y[i];
	}
	for (int i = 0; i < filter_order; i++ )
	{
		state[i] = dataIn[i+n-filter_order];
	}
#else
	for (int i = 0; i < filter_order; i++) {
		x[i] = state[i];
	}

	for (int i = filter_order; i < filter_order + n; i++) {
		x[i] = dataIn[i - filter_order];
	}

	for (int i = 0; i < filter_order + n; i++) {
		y[i] = 0;
		for (int o = 0; o < filter_order; o++) {
			y[i] += coeff[o] * x[i + filter_order - o - 1];
		}
	}
	for (int i = 0; i < n; i++) {
		dataOut[i] = y[i];
	}
	for (int i = 0; i < filter_order; i++) {
		state[i] = dataIn[i + n - filter_order];
	}

#endif

}
