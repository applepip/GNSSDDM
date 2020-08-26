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
 * snPlot.cpp
 *
 *  Created on: Jun 23, 2009
 *      Author: fschubert
 */

const int SETDATA = 1;

#include "snPlot.h"
#include <qwt_legend.h>

snPlot::snPlot(std::string ptitle, YAxisType py_axis_type) :
	title(ptitle), y_axis_type(py_axis_type) {

}

QWidget* snPlot::getWidget() {
	return NULL;
}

void snPlot::replot() {

}

/*void snPlot::setYAxis(double ymin, double ymax) {

 }*/

snPlot::~snPlot() {

}

snPlotQwt::snPlotQwt(std::string ptitle, YAxisType py_axis_type) :
	snPlot(ptitle, py_axis_type) {

}

snPlotQwt::~snPlotQwt() {

}

void snPlotQwt::setXAxis(double xmin, double xmax) {
	plot->setAxisScale(QwtPlot::xBottom, xmin, xmax);
}

void snPlotQwt::setYAxis(double ymin, double ymax) {
	plot->setAxisScale(QwtPlot::yLeft, ymin, ymax);
}

void snPlotQwt::setXAxisLabel(QString l) {
	QwtText Label;
	Label.setText(l);
	Label.setFont(QFont("Helvetica", 10));
	plot->setAxisTitle(QwtPlot::xBottom, Label);
}

void snPlotQwt::setYAxisLabel(QString l) {
	QwtText Label;
	Label.setText(l);
	Label.setFont(QFont("Helvetica", 10));
	plot->setAxisTitle(QwtPlot::yLeft, Label);
}

snPlotQwt3D::snPlotQwt3D(std::string ptitle, YAxisType py_axis_type) :
	snPlot(ptitle, py_axis_type) {

}

snPlotQwt3D::~snPlotQwt3D() {

}

snPlotReal::snPlotReal(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveReal *pc) :
	snPlotQwt(ptitle, py_axis_type) {

	plot = new QwtPlot();

	QwtText PlotTitle;
	PlotTitle.setText(title.c_str());
	PlotTitle.setFont(QFont("Helvetica", 10, QFont::Bold));

	plot->setTitle(PlotTitle);
	plot->setAxisMaxMajor(QwtPlot::xBottom, 3);
	setXAxis(0, pc->nx);
	addCurve(pc);

	//plot->insertLegend(new QwtLegend());
	QwtPlotZoomer *zoomer = new QwtPlotZoomer(plot->canvas());
}

snPlotReal::~snPlotReal() {
	for (size_t i = 0; i < curves.size(); i++) {
		curves.at(i)->CurveI->detach();
	}

	delete plot;
}

void snPlotReal::addCurve(snCurveReal *pc) {
	curves.push_back(pc);
	curves.back()->CurveI->attach(plot);
}

QWidget* snPlotReal::getWidget() {
	return plot;
}

void snPlotReal::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		switch (y_axis_type) {
		case Linear:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = curves.at(o)->Y[i];
			}
			break;
		case Log20:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = 20 * log10(std::abs(curves.at(o)->Y[i]));
			}
			break;
		case Log10:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = 10 * log10(std::abs(curves.at(o)->Y[i]));
			}
			break;
		}
		if (SETDATA)
			curves.at(o)->setData(snCurve::CURVEI);
	}
	plot->replot();
}

snPlotComplexReal::snPlotComplexReal(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurveComplex *pc) :
	snPlotQwt(ptitle, py_axis_type) {

	plot = new QwtPlot();

	QwtText PlotTitle;
	PlotTitle.setText(title.c_str());
	PlotTitle.setFont(QFont("Helvetica", 10, QFont::Bold));
	plot->setTitle(PlotTitle);
	plot->setAxisMaxMajor(QwtPlot::xBottom, 3);
	setXAxis(0, pc->nx);
	addCurve(pc);

	QwtPlotZoomer *zoomer = new QwtPlotZoomer(plot->canvas());
}

snPlotComplexReal::~snPlotComplexReal() {
	for (size_t i = 0; i < curves.size(); i++) {
		curves.at(i)->CurveI->detach();
	}

	delete plot;
}

QWidget* snPlotComplexReal::getWidget() {
	return plot;
}

void snPlotComplexReal::addCurve(snCurveComplex *pc) {
	curves.push_back(pc);
	curves.back()->CurveI->attach(plot);
}

void snPlotComplexReal::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		switch (y_axis_type) {
		case Linear:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = curves.at(o)->Y[i].real();
			}
			break;
		case Log20:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = 20 * log10(abs(curves.at(o)->Y[i]));
			}
			break;
		case Log10:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = 10 * log10(std::abs(curves.at(o)->Y[i]));
			}
			break;
		}
		if (SETDATA)
			curves.at(o)->setData(snCurve::CURVEI);
	}

	plot->replot();
}

snPlotComplexImag::snPlotComplexImag(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurveComplex *pc) :
	snPlotQwt(ptitle, py_axis_type) {

	plot = new QwtPlot();

	QwtText PlotTitle;
	PlotTitle.setText(title.c_str());
	PlotTitle.setFont(QFont("Helvetica", 10, QFont::Bold));
	plot->setTitle(PlotTitle);
	plot->setAxisMaxMajor(QwtPlot::xBottom, 3);
	setXAxis(0, pc->nx);
	addCurve(pc);
	QwtPlotZoomer *zoomer = new QwtPlotZoomer(plot->canvas());

}

snPlotComplexImag::~snPlotComplexImag() {
	for (size_t i = 0; i < curves.size(); i++) {
		curves.at(i)->CurveQ->detach();
	}

	delete plot;
}

QWidget* snPlotComplexImag::getWidget() {
	return plot;
}

void snPlotComplexImag::addCurve(snCurveComplex *pc) {
	curves.push_back(pc);
	curves.back()->CurveQ->attach(plot);
}

void snPlotComplexImag::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		switch (y_axis_type) {
		case Linear:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xQ[i] = curves.at(o)->X[i];
				curves.at(o)->yQ[i] = curves.at(o)->Y[i].imag();
			}
			break;
		case Log20:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xQ[i] = curves.at(o)->X[i];
				curves.at(o)->yQ[i] = 20 * log10(abs(curves.at(o)->Y[i]));
			}
			break;
		case Log10:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xQ[i] = curves.at(o)->X[i];
				curves.at(o)->yQ[i] = 10 * log10(abs(curves.at(o)->Y[i]));
			}
			break;
		}
		if (SETDATA)
			curves.at(o)->setData(snCurve::CURVEQ);
	}
	plot->replot();
}

snPlotComplexRealImag::snPlotComplexRealImag(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurveComplex *pc) :
	snPlotQwt(ptitle, py_axis_type) {

	plot = new QwtPlot();

	QwtText PlotTitle;
	PlotTitle.setText(title.c_str());
	PlotTitle.setFont(QFont("Helvetica", 10, QFont::Bold));
	plot->setTitle(PlotTitle);
	plot->setAxisMaxMajor(QwtPlot::xBottom, 3);
	setXAxis(0, pc->nx);
	addCurve(pc);
	QwtPlotZoomer *zoomer = new QwtPlotZoomer(plot->canvas());

}

snPlotComplexRealImag::~snPlotComplexRealImag() {
	delete plot;
}

snPlotComplexMagn::snPlotComplexMagn(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurveComplex *pc) :
	snPlotComplexReal(ptitle, py_axis_type, pc) {
	setYAxis(-40, 30);
}

snPlotComplexMagn::~snPlotComplexMagn() {

}

void snPlotComplexMagn::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		switch (y_axis_type) {
		case Linear:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = abs(curves.at(o)->Y[i]);
			}
			break;
		case Log20:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = 20 * log10(abs(curves.at(o)->Y[i]));
			}
			break;
		case Log10:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = 10 * log10(abs(curves.at(o)->Y[i]));
			}
			break;
		}
		if (SETDATA)
			curves.at(o)->setData(snCurve::CURVEI);
	}
	plot->replot();
}

snPlotComplexAngle::snPlotComplexAngle(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurveComplex *pc) :
	snPlotComplexImag(ptitle, py_axis_type, pc) {
	setYAxis(-3.5, 3.5);

}

snPlotComplexAngle::~snPlotComplexAngle() {

}

void snPlotComplexAngle::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		switch (y_axis_type) {
		case Linear:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xQ[i] = curves.at(o)->X[i];
				curves.at(o)->yQ[i] = arg(curves.at(o)->Y[i]);
			}
			break;
		case Log20:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xQ[i] = curves.at(o)->X[i];
				curves.at(o)->yQ[i] = 20 * log10(arg(curves.at(o)->Y[i]));
			}
			break;
		case Log10:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xQ[i] = curves.at(o)->X[i];
				curves.at(o)->yQ[i] = 10 * log10(arg(curves.at(o)->Y[i]));
			}
			break;
		}
		if (SETDATA)
			curves.at(o)->setData(snCurve::CURVEQ);
	}
	plot->replot();
}

QWidget* snPlotComplexRealImag::getWidget() {
	return plot;
}

void snPlotComplexRealImag::addCurve(snCurveComplex *pc) {
	curves.push_back(pc);
	curves.back()->CurveI->attach(plot);
	curves.back()->CurveQ->attach(plot);
}

void snPlotComplexRealImag::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		switch (y_axis_type) {
		case Linear:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = curves.at(o)->Y[i].real();
				curves.at(o)->xQ[i] = curves.at(o)->X[i];
				curves.at(o)->yQ[i] = curves.at(o)->Y[i].imag();
			}
			break;
		case Log20:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = 20 * log10(curves.at(o)->Y[i].real());
				curves.at(o)->xQ[i] = curves.at(o)->X[i];
				curves.at(o)->yQ[i] = 20 * log10(curves.at(o)->Y[i].imag());
			}
			break;
		case Log10:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xI[i] = curves.at(o)->X[i];
				curves.at(o)->yI[i] = 10 * log10(curves.at(o)->Y[i].real());
				curves.at(o)->xQ[i] = curves.at(o)->X[i];
				curves.at(o)->yQ[i] = 10 * log10(curves.at(o)->Y[i].imag());
			}
			break;
		}
		if (SETDATA)
			curves.at(o)->setData(snCurve::CURVEI);
		if (SETDATA)
			curves.at(o)->setData(snCurve::CURVEQ);
	}
	plot->replot();
}

snPlotMagn::snPlotMagn(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveComplex *pc,
		double _fsmpl) :
	snPlotQwt(ptitle, py_axis_type), fsmpl(_fsmpl) {

	deltaf = fsmpl / pc->nx;
	freqmin = -1.0 * static_cast<double> (pc->nx) / 2.0 * deltaf;
	freqmax = static_cast<double> (pc->nx) / 2.0 * deltaf;

	// if you have a variable complex<double> *x, you can pass it directly to FFTW via reinterpret_cast<fftw_complex*>(x)
	// http://www.fftw.org/fftw3_doc/Complex-numbers.html#Complex-numbers

	plot = new QwtPlot();

	QwtText PlotTitle;
	PlotTitle.setText(title.c_str());
	PlotTitle.setFont(QFont("Helvetica", 10, QFont::Bold));
	plot->setTitle(PlotTitle);
	plot->setAxisMaxMajor(QwtPlot::xBottom, 3);
	setYAxis(-40, 30);
	//	setXAxis(0, pc->nx);
	setXAxis(freqmin, freqmax);

	QwtText LabelX;
	LabelX.setText("f [Hz]");
	LabelX.setFont(QFont("Helvetica", 10));
	plot->setAxisTitle(QwtPlot::xBottom, LabelX);

	QwtText LabelY;
	LabelY.setText("Magnitude");
	LabelY.setFont(QFont("Helvetica", 10));
	plot->setAxisTitle(QwtPlot::yLeft, LabelY);

	addCurve(pc);
	QwtPlotZoomer *zoomer = new QwtPlotZoomer(plot->canvas());

}

snPlotMagn::~snPlotMagn() {
	for (size_t i = 0; i < curves.size(); i++) {
		curves.at(i)->CurveSM->detach();
	}

	delete plot;
}

QWidget* snPlotMagn::getWidget() {
	return plot;
}

void snPlotMagn::addCurve(snCurveComplex *pc) {
	curves.push_back(pc);
	curves.back()->CurveSM->attach(plot);
}

void snPlotMagn::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		curves.at(o)->runFFT();
		switch (y_axis_type) {
		case Linear:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xSM[i] = static_cast<double> (i) * deltaf + freqmin;
				curves.at(o)->ySM[i] = curves.at(o)->fft_spec[i];
			}
			break;
		case Log20:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xSM[i] = static_cast<double> (i) * deltaf + freqmin;
				curves.at(o)->ySM[i] = 20 * log10(curves.at(o)->fft_spec[i]);
			}
			break;
		case Log10:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xSM[i] = static_cast<double> (i) * deltaf + freqmin;
				curves.at(o)->ySM[i] = 10 * log10(curves.at(o)->fft_spec[i]);
			}
			break;
		}
		if (SETDATA)
			curves.at(o)->setData(snCurve::CURVESM);
	}
	plot->replot();

}

snPlotAngl::snPlotAngl(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveComplex *pc,
		double _fsmpl) :
	snPlotQwt(ptitle, py_axis_type), fsmpl(_fsmpl) {

	deltaf = fsmpl / pc->nx;
	freqmin = -1.0 * static_cast<double> (pc->nx) / 2.0 * deltaf;
	freqmax = static_cast<double> (pc->nx) / 2.0 * deltaf;

	plot = new QwtPlot();

	QwtText PlotTitle;
	PlotTitle.setText(title.c_str());
	PlotTitle.setFont(QFont("Helvetica", 10, QFont::Bold));
	plot->setTitle(PlotTitle);
	plot->setAxisMaxMajor(QwtPlot::xBottom, 3);
	//	setXAxis(0, pc->nx);
	setXAxis(freqmin, freqmax);
	setYAxis(-3.5, 3.5);

	QwtText LabelX;
	LabelX.setText("f [Hz]");
	LabelX.setFont(QFont("Helvetica", 10));
	plot->setAxisTitle(QwtPlot::xBottom, LabelX);

	QwtText LabelY;
	LabelY.setText("Angle");
	LabelY.setFont(QFont("Helvetica", 10));
	plot->setAxisTitle(QwtPlot::yLeft, LabelY);

	addCurve(pc);
	QwtPlotZoomer *zoomer = new QwtPlotZoomer(plot->canvas());

}

snPlotAngl::~snPlotAngl() {
	for (size_t i = 0; i < curves.size(); i++) {
		curves.at(i)->CurveSA->detach();
	}

	delete plot;
}

QWidget* snPlotAngl::getWidget() {
	return plot;
}

void snPlotAngl::addCurve(snCurveComplex *pc) {
	curves.push_back(pc);
	curves.back()->CurveSA->attach(plot);
}

void snPlotAngl::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		curves.at(o)->runFFT();
		switch (y_axis_type) {
		case Linear:
			for (int i = 0; i < curves.at(o)->nx; i++) {
				curves.at(o)->xSA[i] = static_cast<double> (i) * deltaf + freqmin;
				;
				curves.at(o)->ySA[i] = curves.at(o)->fft_angl[i];
			}
			if (SETDATA)
				curves.at(o)->setData(snCurve::CURVESA);
			plot->replot();
			break;
		case Log20:
		case Log10:
			std::cout << "snPlotAngl: parameter Log for plotting angles does not make sense.\n";
			std::exit(1);
			break;
		}
	}
}

snPlotSurface::snPlotSurface(std::string ptitle, snPlot::YAxisType py_axis_type, snCurve3D *pc) :
	snPlotQwt3D(ptitle, py_axis_type), curve(pc) {

	plot = new Qwt3D::SurfacePlot();
	plot->setPlotStyle(Qwt3D::FILLEDMESH);
	plot->setMeshLineWidth(0.001);
	plot->setCoordinateStyle(Qwt3D::FRAME);
	plot->setRotation(45, 0, 45);
	plot->setScale(1, 5, 100);
	//	plot->setZoom(0.2);
}

snPlotSurface::~snPlotSurface() {
	delete plot;
}

QWidget* snPlotSurface::getWidget() {
	return plot;
}

void snPlotSurface::replot() {
	plot->loadFromData(curve->SXY, curve->nx, curve->ny, 0, curve->nx, curve->ny, 0);
	plot->updateData();
	plot->update();
}
