/*
 *  SNACS - The Satellite Navigation Radio Channel Simulator
 *
 *  Copyright (C) 2012  F. M. Schubert
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
 * snPlotQwt.h
 *
 *      Author: Frank Schubert
 */

#ifndef SNPLOTQWT_H_
#define SNPLOTQWT_H_

#include <iostream>
#include <tr1/memory>

#include <QWidget>

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include <qwt3d_surfaceplot.h>

#include "../snWidget/snPlot.h"

/*
 * Implements a plotting class using the Qwt widget set for Qt.
 */
class snPlotQwt: public snPlot {
public:
	snPlotQwt(std::string ptitle, YAxisType py_axis_type);
	virtual ~snPlotQwt();
	QWidget* getWidget() {
		return plot.get();
	}

	void setYAxis(double ymin, double ymax);
	void setXAxis(double xmin, double xmax);
	void setXAxisLabel(std::string l);
	void setYAxisLabel(std::string l);

	void enableGrid() {
		QwtPlotGrid *grid = new QwtPlotGrid();
		grid->attach(plot.get());
	}

	QColor get_QColor_from_CurveColor(snCurve::CurveColor color);
	QwtPlotCurve::CurveStyle get_qwt_style_from_CurveStyle(snCurve::CurveStyle style);
	QwtSymbol get_qwt_symbol_from_CurveSymbol(snCurve::CurveSymbol symbol, snCurve::CurveColor color,
			snCurve::SymbolSize size);
	void add_qwt_curve(std::string _name, size_t _max_points, snCurve::CurveColor _color,
			snCurve::CurveStyle _style, snCurve::CurveSymbol _symbol, snCurve::SymbolSize _symsize);

	void enableLegend();

	void copy_vec_to_xI(const itpp::vec &v, size_t o);
	void copy_vec_to_yI(const itpp::vec &v, size_t o);

protected:
	std::tr1::shared_ptr<QwtPlot> plot;

	typedef std::tr1::shared_ptr<QwtPlotCurve> QwtCurvePtr;
	std::vector<QwtCurvePtr> qwt_curves;

	std::tr1::shared_ptr<QwtLegend> qwt_legend;
	std::vector<double *> xI; ///< holds the raw data for qwt_plot
	std::vector<double *> yI; ///< holds the raw data for qwt_plot
	std::vector<int> data_lengths; ///< keeps track of the length of xI pointers
};

/*
 * Implements a plotting class for real data
 */
class snPlotRealQwt: public snPlotQwt {
public:
	snPlotRealQwt(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveReal *_curve);
	virtual ~snPlotRealQwt();
	void replot();
	void addCurve(snCurveReal *pc);
	void addCurve(snCurveComplex *pc) {
		std::cerr << "error: snPlotRealQwt::addCurve cannot process snCurveComplex for " << pc->get_name()
				<< "\n";
		std::abort();
	}
	void addCurve(snCurve3D *pc) {
		std::cerr << "error: snPlotRealQwt::addCurve cannot process snCurve3D for " << pc->get_name() << "\n";
		std::abort();
	}

protected:
	std::vector<snCurveReal *> curves;
};

/*
 * Implements a plotting class for complex data
 */
class snPlotComplexQwt: public snPlotQwt {
public:
	enum DataType {
		real, imag
	};

	snPlotComplexQwt(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveComplex *_curve);
	virtual ~snPlotComplexQwt();
	QWidget* getWidget();
	void replot();
	void addCurve(snCurveComplex *pc);
	void addCurve(snCurveReal *pc) {
		std::cerr << "error: snPlotComplexQwt::addCurve cannot process snCurveReal for " << pc->get_name()
				<< "\n";
		std::abort();
	}
	void addCurve(snCurve3D *pc) {
		std::cerr << "error: snPlotComplexQwt::addCurve cannot process snCurve3D for " << pc->get_name()
				<< "\n";
		std::abort();
	}
protected:
	std::vector<snCurveComplex *> curves;
	std::vector<DataType> data_types;
	double smpl_freq;
};

/*
 * Implements a plotting class for 3d data
 */
class snPlotSurfaceQwt: public snPlot {
public:
	snPlotSurfaceQwt(std::string ptitle, snPlot::YAxisType py_axis_type, snCurve3D *pc);
	virtual ~snPlotSurfaceQwt();
	void replot();
	QWidget* getWidget() {
		return plot.get();
	}
	void setYAxis(double ymin, double ymax) {
		plot->coordinates()->axes[1].setLimits(ymin, ymax);
	}
	void setXAxis(double xmin, double xmax) {
		plot->coordinates()->axes[0].setLimits(xmin, xmax);
	}
	void setXAxisLabel(std::string l) {
		plot->coordinates()->axes[0].setLabelString(l.c_str());
	}
	void setYAxisLabel(std::string l) {
		plot->coordinates()->axes[1].setLabelString(l.c_str());
	}
	void enableGrid() {
	}
	void enableLegend() {
	}
	void addCurve(snCurveReal *pc) {
		std::cerr << "error: snPlotSurfaceQwt::addCurve cannot process snCurveReal for " << pc->get_name()
				<< "\n";
		std::abort();
	}
	void addCurve(snCurveComplex *pc) {
		std::cerr << "error: snPlotSurfaceQwt::addCurve cannot process snCurveComplex for " << pc->get_name()
				<< "\n";
		std::abort();
	}
	void addCurve(snCurve3D *_curve);

private:
	std::tr1::shared_ptr<Qwt3D::SurfacePlot> plot;
	std::tr1::shared_ptr<snCurve3D> curve;
};

#endif
