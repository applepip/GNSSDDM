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

/*
 * snCurve.cpp
 *
 *  Created on: Jun 23, 2009
 *      Author: fschubert
 */

#include "snCurve.h"

	void snCurve::setData() {
	//std::cout << "set1 nx: " << nx << std::endl;
			CurveI->setData(xI, yI, nx);
			CurveQ->setData(xQ, yQ, nx);
			CurveSM->setData(xSM, ySM, nx);
			CurveSA->setData(xSA, ySA, nx);
	}

void snCurve::setData(CurveType ct) {
	//std::cout <<"set2 nx: " << nx << std::endl;
	switch (ct) {
		case CURVEI:
			CurveI->setData(xI, yI, nx);
		break;
		case CURVEQ:
			CurveQ->setData(xQ, yQ, nx);
		break;
		case CURVESM:
			CurveSM->setData(xSM, ySM, nx);
		break;
		case CURVESA:
			CurveSA->setData(xSA, ySA, nx);
		break;
		}
	}

		void snCurve::setPointsToPlot(CurveType ct, int p) {
		switch (ct) {
		case CURVEI:
			//CurveI->setRawData(xI, yI, p);
			break;
		case CURVEQ:
			//CurveQ->setRawData(xQ, yQ, p);
			break;
		case CURVESM:
			//CurveSM->setRawData(xQ, yQ, p);
			break;
		case CURVESA:
			//CurveSA->setRawData(xQ, yQ, p);
			break;
		}
	}

	
snCurve::snCurve(int pn, QColor pcolor) :
	nx(pn), color(pcolor) {

	//X = new double[nx];
	X.resize(nx);

	xI = new double[nx];
	yI = new double[nx];
	xSM = new double[nx];
	ySM = new double[nx];
	xSA = new double[nx];
	ySA = new double[nx];

	CurveI = new QwtPlotCurve();
	CurveI->setPen(QPen(color));
	//CurveI->setRawData(xI, yI, nx);
	CurveI->setCurveAttribute(QwtPlotCurve::Fitted, false);
	
	xQ = new double[nx];
	yQ = new double[nx];

	CurveQ = new QwtPlotCurve();
	CurveQ->setPen(QPen(color));
	//CurveQ->setRawData(xQ, yQ, nx);
	CurveQ->setCurveAttribute(QwtPlotCurve::Fitted, false);

	CurveSM = new QwtPlotCurve();
	CurveSM->setPen(QPen(Qt::darkGreen)); // set standard color for spectrum to green
	//CurveSM->setRawData(xSM, ySM, nx);
	CurveSM->setCurveAttribute(QwtPlotCurve::Fitted, false);

	CurveSA = new QwtPlotCurve();
	CurveSA->setPen(QPen(Qt::darkCyan)); // set standard color for phase to pink
	//CurveSA->setRawData(xSA, ySA, nx);
	CurveSA->setCurveAttribute(QwtPlotCurve::Fitted, false);
}

snCurve::~snCurve() {
	delete CurveI;
	delete CurveSM;
	delete CurveSA;

	delete CurveQ;

	delete[] xQ;
	delete[] yQ;

	//delete[] X;
	delete[] xI;
	delete[] yI;
	delete[] xSM;
	delete[] ySM;
	delete[] xSA;
	delete[] ySA;
}

void snCurve::setCurveStyle(CurveType ct, QwtPlotCurve::CurveStyle style) {
	switch (ct) {
	case CURVEI:
		CurveI->setStyle(style);
		break;
	case CURVEQ:
		CurveQ->setStyle(style);
		break;
	case CURVESM:
		CurveSM->setStyle(style);
		break;
	case CURVESA:
		CurveSA->setStyle(style);
		break;
	}
}

void snCurve::setCurveSymbol(CurveType ct, QwtSymbol sym) {
	switch (ct) {
	case CURVEI:
		CurveI->setSymbol(sym);
		break;
	case CURVEQ:
		CurveQ->setSymbol(sym);
		break;
	case CURVESM:
		CurveSM->setSymbol(sym);
		break;
	case CURVESA:
		CurveSA->setSymbol(sym);
		break;
	}
}

snCurve3D::snCurve3D(int pnx, int pny) :
	snCurve(pnx, Qt::black), ny(pny) {

	SXY = new double*[nx];
	for (int i = 0; i < nx; i++) {
		SXY[i] = new double[ny];
	}
}

snCurve3D::~snCurve3D() {
	for (int i = 0; i < nx; i++) {
		delete[] SXY[i];
	}
	delete[] SXY;
}

snCurveReal::snCurveReal(int pn, QColor pcolor) :
	snCurve(pn, pcolor) {
	//Y = new double[nx];
	Y.resize(nx);

}

snCurveReal::~snCurveReal() {
	//delete[] Y;
}

snCurveComplex::snCurveComplex(int pn, QColor pcolorI, QColor pcolorQ) :
	snCurve(pn, pcolorI) {
	//Y = new std::complex<double>[nx];
	Y.resize(nx);


	CurveQ->setPen(QPen(pcolorQ));
	
	FFTout = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nx);
	fftwplan = fftw_plan_dft_1d(nx, reinterpret_cast<fftw_complex*> (&Y[0]),
			FFTout, FFTW_FORWARD, FFTW_ESTIMATE);
	FFT = new std::complex<double>[nx];
	fft_spec = new double[nx];
	fft_angl = new double[nx];
}

snCurveComplex::~snCurveComplex() {
	delete[] fft_angl;
	delete[] fft_spec;
	delete[] FFT;
	fftw_destroy_plan(fftwplan);
	fftw_free(FFTout);

	//delete[] Y;
}

void snCurveComplex::runFFT() {
	fftw_execute(fftwplan);
	for (int i = 0; i < nx / 2; i++) {
		FFT[i].real() = FFTout[i + nx / 2][0];
		FFT[i].imag() = FFTout[i + nx / 2][1];
	}
	for (int i = nx / 2; i < nx; i++) {
		FFT[i].real() = FFTout[i - nx / 2][0];
		FFT[i].imag() = FFTout[i - nx / 2][1];
	}
	for (int i = 0; i < nx; i++) {
		fft_spec[i] = pow(std::abs(FFT[i]), 2) / static_cast<double> (nx);
		fft_angl[i] = arg(FFT[i]);
	}

	//	itpp::vec spec_in(nx);
	//	itpp::vec out(nx);
	//	for (int i = 0; i < nx; i++) {
	//		spec_in[i] = Y[i].real();
	//	}
	//	out = itpp::spectrum(spec_in, 128, 1);
	//	for (int i = 0; i < out.size(); i++) {
	//		FFT[i].real() = out[i];
	//		FFT[i].imag() = 0.0;
	//	}

}
