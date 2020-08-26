/*
 *  SNACS - The Satellite Navigation Radio Channel Simulator
 *
 *  Copyright (C) 2012 F. M. Schubert
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

/*
 * snCurve.cpp
 *
 *  Created on: Jun 23, 2009
 *      Author: fschubert
 */

#include "snCurve.h"

#include <itpp/signal/transforms.h>

snCurve::snCurve(int pn, CurveColor pcolor, std::string _name) :
		max_points(pn), points_to_plot(pn), color(pcolor), name(_name) {

	X.set_size(max_points);
	X.zeros();
}

snCurve::~snCurve() {
}

void snCurve::setPointsToPlot(size_t p) {
	if (p <= max_points)
		points_to_plot = p;
	else
		points_to_plot = max_points;
}

itpp::vec snCurve::get_frequency_vector() {
	// get smpl freq:
	const double interval = (X(X.size() - 1) - X(0)) / static_cast<double>(X.size());
	const double smpl_freq = 1. / interval;
//	std::cout << "X(0): " << X(0) << ", X.get(-1): " << X.get(-1) << ", X(X.size()-1): " << X(X.size() - 1)
//			<< ", interval : " << interval << ", smpl_freq : " << smpl_freq << "\n";
	const double deltaf = smpl_freq / X.size();
	const double freqmin = -1.0 * static_cast<double>(X.size()) / 2.0 * deltaf;

	itpp::vec freq_vec(X.size());
	for (int i = 0; i < X.size(); i++) {
		freq_vec(i) = freqmin + i * deltaf;
	}
	//const double freqmax = static_cast<double>(curves.at(o)->get_max_points()) / 2.0 * deltaf;
	//			setXAxis(freqmin, freqmax);
	return freq_vec;
}

itpp::cvec snCurve::fft(const itpp::cvec &fft_in) {
	const itpp::cvec fft_out = itpp::fft(fft_in);
	const size_t mid_idx = floor(fft_out.length() / 2.);
	const itpp::cvec fft_out_shift = itpp::concat(fft_out.right(mid_idx), fft_out.left(mid_idx));
	return fft_out_shift;
}

snCurveReal::snCurveReal(int pn, CurveColor pcolor, std::string _name) :
		snCurve(pn, pcolor, _name) {
	Y.set_size(max_points);
	Y.zeros();
}

void snCurveReal::update_value_Y(size_t i, double v) {
	Y(i) = v;
}

snCurveReal::~snCurveReal() {
}


itpp::vec snCurveReal::fft_magnitude() {
	return itpp::abs(fft(itpp::to_cvec(Y)));
}

itpp::vec snCurveReal::fft_phase() {
	return itpp::arg(fft(itpp::to_cvec(Y)));
}

snCurveComplex::snCurveComplex(int pn, CurveColor pcolorI, CurveColor pcolorQ, std::string _name) :
		snCurve(pn, pcolorI, _name), colorQ(pcolorQ) {
	Y.set_size(max_points);
	Y.zeros();
}

snCurveComplex::~snCurveComplex() {
}

void snCurveComplex::update_value_X(size_t i, double v) {
	X(i) = v;
}

void snCurveComplex::update_value_Y(size_t i, std::complex<double> v) {
	Y(i) = v;
}

itpp::vec snCurveComplex::fft_magnitude() {
	return itpp::abs(fft(Y));
}

itpp::vec snCurveComplex::fft_phase() {
	return itpp::arg(fft(Y));
}

snCurve3D::snCurve3D(int pnx, int pny) :
		snCurve(pnx, snCurve::black), ny(pny) {

	SXY = new double*[max_points];
	for (size_t i = 0; i < max_points; i++) {
		SXY[i] = new double[ny];
	}
}

snCurve3D::~snCurve3D() {
	for (size_t i = 0; i < max_points; i++) {
		delete[] SXY[i];
	}
	delete[] SXY;
}

void snCurve3D::update_XY(size_t ix, size_t iy, double v) {
	SXY[ix][iy] = v;
}
