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

#include "snPlotQwt.h"

snPlotQwt::snPlotQwt(std::string ptitle, YAxisType py_axis_type) :
		snPlot(ptitle, py_axis_type) {
	plot.reset(new QwtPlot());

	QwtText PlotTitle;
	PlotTitle.setText(title.c_str());
	PlotTitle.setFont(QFont("Helvetica", 10, QFont::Bold));
	plot->setTitle(PlotTitle);
	plot->setAxisMaxMajor(QwtPlot::xBottom, 3);

	//QwtPlotZoomer *zoomer = new QwtPlotZoomer(plot->canvas());

}

snPlotQwt::~snPlotQwt() {
	for (size_t i = 0; i < qwt_curves.size(); i++) {
		qwt_curves.at(i)->detach();
	}

	for (size_t k = 0; k < xI.size(); k++)
		delete[] xI.at(k);

	for (size_t k = 0; k < yI.size(); k++)
		delete[] yI.at(k);
}

void snPlotQwt::setXAxis(double xmin, double xmax) {
	plot->setAxisScale(QwtPlot::xBottom, xmin, xmax);
}

void snPlotQwt::setYAxis(double ymin, double ymax) {
	plot->setAxisScale(QwtPlot::yLeft, ymin, ymax);
}

void snPlotQwt::setXAxisLabel(std::string l) {
	QwtText Label;
	Label.setText(l.c_str());
	Label.setFont(QFont("Helvetica", 10));
	plot->setAxisTitle(QwtPlot::xBottom, Label);
}

void snPlotQwt::setYAxisLabel(std::string l) {
	QwtText Label;
	Label.setText(l.c_str());
	Label.setFont(QFont("Helvetica", 10));
	plot->setAxisTitle(QwtPlot::yLeft, Label);
}

void snPlotQwt::enableLegend() {
	qwt_legend.reset(new QwtLegend());
	plot->insertLegend(qwt_legend.get(), QwtPlot::RightLegend);
}

QColor snPlotQwt::get_QColor_from_CurveColor(snCurve::CurveColor color) {
	QColor qc;
	switch (color) {
	case snCurve::black:
		qc = Qt::black;
		break;
	case snCurve::blue:
		qc = Qt::blue;
		break;
	case snCurve::darkBlue:
		qc = Qt::darkBlue;
		break;
	case snCurve::red:
		qc = Qt::red;
		break;
	case snCurve::darkRed:
		qc = Qt::darkRed;
		break;
	case snCurve::green:
		qc = Qt::green;
		break;
	case snCurve::darkGreen:
		qc = Qt::darkGreen;
		break;
	}
	return qc;
}

void snPlotQwt::add_qwt_curve(std::string _name, size_t _max_points, snCurve::CurveColor _color,
		snCurve::CurveStyle _style, snCurve::CurveSymbol _symbol, snCurve::SymbolSize _symsize) {

	// get color
	const QColor qcolor = get_QColor_from_CurveColor(_color);

	QwtCurvePtr new_qwt_curve(new QwtPlotCurve(_name.c_str()));
	qwt_curves.push_back(new_qwt_curve);
	qwt_curves.back()->setPen(QPen(qcolor));
	qwt_curves.back()->setCurveAttribute(QwtPlotCurve::Fitted, false);

	// get style
	qwt_curves.back()->setStyle(get_qwt_style_from_CurveStyle(_style));

	// get symbol
	qwt_curves.back()->setSymbol(get_qwt_symbol_from_CurveSymbol(_symbol, _color, _symsize));

	qwt_curves.back()->attach(plot.get());

	xI.push_back(new double[_max_points]);
	yI.push_back(new double[_max_points]);
	data_lengths.push_back(_max_points);
}

void snPlotQwt::copy_vec_to_xI(const itpp::vec &v, size_t o) {
	if (v.size() > data_lengths.at(o)) {
		std::cout << "copy_vec_to_array: v.size() > data_lengths.at(o).\n";
		std::abort();
	}
	for (int i = 0; i < v.size(); i++)
		xI.at(o)[i] = v(i);
}

void snPlotQwt::copy_vec_to_yI(const itpp::vec &v, size_t o) {
	if (v.size() > data_lengths.at(o)) {
		std::cout << "copy_vec_to_array: v.size() > data_lengths.at(o).\n";
		std::abort();
	}
	for (int i = 0; i < v.size(); i++)
		yI.at(o)[i] = v(i);
}

QwtPlotCurve::CurveStyle snPlotQwt::get_qwt_style_from_CurveStyle(snCurve::CurveStyle style) {
	QwtPlotCurve::CurveStyle qwtstyle = QwtPlotCurve::Lines;

	switch (style) {
	case snCurve::LINES:
		qwtstyle = QwtPlotCurve::Lines;
		break;
	case snCurve::STICKS:
		qwtstyle = QwtPlotCurve::Sticks;
		break;
	case snCurve::DOTS:
		qwtstyle = QwtPlotCurve::Dots;
		break;
	}
	return qwtstyle;
}

QwtSymbol snPlotQwt::get_qwt_symbol_from_CurveSymbol(snCurve::CurveSymbol symbol, snCurve::CurveColor color,
		snCurve::SymbolSize size) {

	QwtSymbol qwtsymbol = QwtSymbol(QwtSymbol::NoSymbol, QBrush(color), QPen(color), QSize(3, 3));

	int psize = 0;
	switch (size) {
	case snCurve::SMALL:
		psize = 3;
		break;
	case snCurve::MEDIUM:
		psize = 5;
		break;
	case snCurve::LARGE:
		psize = 8;
		break;
	}

	QColor qcolor = get_QColor_from_CurveColor(color);
	switch (symbol) {
	case snCurve::NONE:
		qwtsymbol = QwtSymbol(QwtSymbol::NoSymbol, QBrush(qcolor), QPen(qcolor), QSize(psize, psize));
		break;
	case snCurve::CIRCLE:
		qwtsymbol = QwtSymbol(QwtSymbol::Ellipse, QBrush(qcolor), QPen(qcolor), QSize(psize, psize));
		break;
	}
	return qwtsymbol;
}

snPlotRealQwt::snPlotRealQwt(std::string ptitle, snPlotQwt::YAxisType py_axis_type, snCurveReal *_curve) :
		snPlotQwt(ptitle, py_axis_type) {

	addCurve(_curve);
}

snPlotRealQwt::~snPlotRealQwt() {
}

void snPlotRealQwt::addCurve(snCurveReal *pc) {
	add_qwt_curve(pc->get_name(), pc->get_max_points(), pc->get_color(), pc->get_style(), pc->get_symbol(),
			pc->get_symbol_size());
	curves.push_back(pc);
}

void snPlotRealQwt::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		switch (y_axis_type) {
		case Linear:
			for (size_t i = 0; i < curves.at(o)->get_max_points(); i++) {
				xI.at(o)[i] = curves.at(o)->get_X(i);
				yI.at(o)[i] = curves.at(o)->get_Y(i);
			}
			break;
		case Log20:
			for (size_t i = 0; i < curves.at(o)->get_max_points(); i++) {
				xI.at(o)[i] = curves.at(o)->get_X(i);
				yI.at(o)[i] = 20 * log10(std::abs(curves.at(o)->get_Y(i)));
			}
			break;
		case Log10:
			for (size_t i = 0; i < curves.at(o)->get_max_points(); i++) {
				xI.at(o)[i] = curves.at(o)->get_X(i);
				yI.at(o)[i] = 10 * log10(std::abs(curves.at(o)->get_Y(i)));
			}
			break;
		case FFTMagnitudeDB: {
			const itpp::vec freq_vec = curves.at(o)->get_frequency_vector();
			copy_vec_to_xI(freq_vec, o);
			const itpp::vec fft_magn = 20 * itpp::log10(curves.at(o)->fft_magnitude());
			copy_vec_to_yI(fft_magn, o);
		}
			break;
		case FFTAngle: {
			const itpp::vec freq_vec = curves.at(o)->get_frequency_vector();
			copy_vec_to_xI(freq_vec, o);
			const itpp::vec fft_angl = curves.at(o)->fft_phase();
			copy_vec_to_yI(fft_angl, o);
		}
			break;
		case Angle:
			std::cerr << "snPlotRealQwt::replot(): plotting angle is not defined for real curve.\n";
			std::abort();
			break;

		}
		qwt_curves.at(o)->setData(xI.at(o), yI.at(o), curves.at(o)->get_points_to_plot());
	}
	plot->replot();
}

snPlotComplexQwt::snPlotComplexQwt(std::string ptitle, snPlotQwt::YAxisType py_axis_type,
		snCurveComplex *_curve) :
		snPlotQwt(ptitle, py_axis_type) {

	addCurve(_curve);
}

snPlotComplexQwt::~snPlotComplexQwt() {
}

void snPlotComplexQwt::addCurve(snCurveComplex *pc) {
	// I
	add_qwt_curve(pc->get_name(), pc->get_max_points(), pc->get_color(), pc->get_style(), pc->get_symbol(),
			pc->get_symbol_size());
	curves.push_back(pc);
	data_types.push_back(snPlotComplexQwt::real);

	// Q
	add_qwt_curve(pc->get_name(), pc->get_max_points(), pc->get_colorQ(), pc->get_style(), pc->get_symbol(),
			pc->get_symbol_size());
	curves.push_back(pc);
	data_types.push_back(snPlotComplexQwt::imag);
}

void snPlotComplexQwt::replot() {
	for (size_t o = 0; o < curves.size(); o++) {
		switch (y_axis_type) {
		case Linear:
			for (size_t i = 0; i < curves.at(o)->get_max_points(); i++) {
				xI.at(o)[i] = curves.at(o)->get_X(i);
				switch (data_types.at(o)) {
				case real:
					yI.at(o)[i] = curves.at(o)->get_Y(i).real();
					break;
				case imag:
					yI.at(o)[i] = curves.at(o)->get_Y(i).imag();
					break;
				}
				//std::cout << "curves.at(o)->yI[i]: " << curves.at(o)->yI[i] << "\n";
			}
			break;
		case Log20:
			for (size_t i = 0; i < curves.at(o)->get_max_points(); i++) {
				xI.at(o)[i] = curves.at(o)->get_X(i);
				yI.at(o)[i] = 20 * log10(abs(curves.at(o)->get_Y(i)));
			}
			break;
		case Log10:
			for (size_t i = 0; i < curves.at(o)->get_max_points(); i++) {
				xI.at(o)[i] = curves.at(o)->get_X(i);
				yI.at(o)[i] = 10 * log10(abs(curves.at(o)->get_Y(i)));
			}
			break;
		case FFTMagnitudeDB: {
			const itpp::vec freq_vec = curves.at(o)->get_frequency_vector();
			copy_vec_to_xI(freq_vec, o);
			const itpp::vec fft_magn = 20 * itpp::log10(curves.at(o)->fft_magnitude());
			copy_vec_to_yI(fft_magn, o);
		}
			break;
		case FFTAngle: {
			const itpp::vec freq_vec = curves.at(o)->get_frequency_vector();
			copy_vec_to_xI(freq_vec, o);
			const itpp::vec fft_angl = curves.at(o)->fft_phase();
			copy_vec_to_yI(fft_angl, o);
		}
			break;
		case Angle:
			for (size_t i = 0; i < curves.at(o)->get_max_points(); i++) {
				xI.at(o)[i] = curves.at(o)->get_X(i);
				yI.at(o)[i] = arg(curves.at(o)->get_Y(i));
			}
			break;
		}
		qwt_curves.at(o)->setData(xI.at(o), yI.at(o), curves.at(o)->get_points_to_plot());

	}
	plot->replot();
}

snPlotSurfaceQwt::snPlotSurfaceQwt(std::string ptitle, snPlot::YAxisType py_axis_type, snCurve3D *pc) :
		snPlot(ptitle, py_axis_type), curve(pc) {

	plot.reset(new Qwt3D::SurfacePlot());
	plot->setPlotStyle(Qwt3D::FILLEDMESH);
	plot->setMeshLineWidth(0.001);
	plot->setCoordinateStyle(Qwt3D::FRAME);
	plot->setRotation(45, 0, 45);
	plot->setScale(1, 5, 100);
//	plot->setZoom(0.2);
}

snPlotSurfaceQwt::~snPlotSurfaceQwt() {
}

void snPlotSurfaceQwt::replot() {
	plot->loadFromData(curve->get_SXY(), curve->get_max_points(), curve->ny, 0, curve->get_max_points(),
			curve->ny, 0);
	plot->updateData();
	plot->update();
}

void snPlotSurfaceQwt::addCurve(snCurve3D *_curve) {

}
