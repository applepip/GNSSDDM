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

/*!
 * \file	snCurve.h
 * \brief	Contains implementations for snCurves to be used with snPlots.
 *
 * \author	Frank Schubert
 */

#ifndef SNCURVE_H_
#define SNCURVE_H_

#include <complex>

#include <iostream>

#include <QColor>

#include <fftw3.h>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

/*!
 * \brief	snCurve is the base class for all curves that can be used with snPlots
 *
 *
 */
class snCurve {
public:
	enum CurveType {
		CURVEI, CURVEQ, CURVESM, CURVESA
	};

	snCurve(int pn, QColor pcolor);
	~snCurve();

	void setData();
	void setData(CurveType ct);
	
	int nx;
	QColor color;
	//double *X; ///< holds x axis component
	std::vector<double> X; ///< holds x axis component

	double *xI, *yI;
	double *xQ, *yQ;
	double *xSM, *ySM;
	double *xSA, *ySA;

	QwtPlotCurve *CurveI, *CurveQ, *CurveSM, *CurveSA;

	void setCurveStyle(CurveType ct, QwtPlotCurve::CurveStyle style);
	void setCurveSymbol(CurveType ct, QwtSymbol sym);
	void setPointsToPlot(CurveType ct, int p);
};

/*!
 * \brief	snCurveReal implements a curve for real data
 *
 * \param
 *
 * \return
 * \retval
 */

class snCurveReal: public snCurve {
public:
	snCurveReal(int pn, QColor pcolor);
	~snCurveReal();
	//double *Y;
	std::vector<double> Y;
};

class snCurveComplex: public snCurve {
public:
	snCurveComplex(int pn, QColor pcolorI, QColor pcolorQ);
	~snCurveComplex();

	std::vector<std::complex<double> > Y; ///< holds signal inphase and quadrature component

	void runFFT();
	fftw_complex *FFTout;
	fftw_plan fftwplan;
	std::complex<double> *FFT; ///< holds signal inphase and quadrature component
	double *fft_spec, *fft_angl; ///< holds signal inphase and quadrature component
};

class snCurve3D: public snCurve {
public:
	snCurve3D(int pnx, int pny);
	~snCurve3D();

	int ny;
	double **SXY; ///< holds 3D data
private:
};

#endif /* SNCURVE_H_ */
