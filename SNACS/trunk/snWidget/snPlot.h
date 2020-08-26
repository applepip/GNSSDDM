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
 * snPlot.h
 *
 *  Created on: Jun 23, 2009
 *      Author: fschubert
 */

#ifndef SNPLOT_H_
#define SNPLOT_H_

#include <iostream>

#include <QWidget>

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt3d_surfaceplot.h>
#include <qwt_plot_zoomer.h>

#include "snCurve.h"

class snPlot {
public:
	enum YAxisType {
		Linear, Log20, Log10
	};

	snPlot(std::string ptitle, YAxisType py_axis_type);
	virtual ~snPlot();
	virtual QWidget* getWidget();
	virtual void replot();

	virtual void setYAxis(double, double) {

	}
protected:
	std::string title;
	YAxisType y_axis_type;
};

class snPlotQwt: public snPlot {
public:
	snPlotQwt(std::string ptitle, YAxisType py_axis_type);
	virtual ~snPlotQwt();
	void setYAxis(double ymin, double ymax);
	void setXAxis(double xmin, double xmax);
	void setXAxisLabel(QString l);
	void setYAxisLabel(QString l);

	void enableGrid() {
		QwtPlotGrid *grid = new QwtPlotGrid();
		grid->attach(plot);
	}

protected:
	QwtPlot *plot;

};

class snPlotQwt3D: public snPlot {
public:
	snPlotQwt3D(std::string ptitle, YAxisType py_axis_type);
	virtual ~snPlotQwt3D();
protected:
	Qwt3D::SurfacePlot *plot;
	snCurve3D *curve;
};

class snPlotReal: public snPlotQwt {
public:
	snPlotReal(std::string ptitle, snPlot::YAxisType py_axis_type,
			snCurveReal *pc);
	virtual ~snPlotReal();
	QWidget* getWidget();
	void replot();
	void addCurve(snCurveReal *pc);
protected:
	std::vector<snCurveReal *> curves;
};

class snPlotComplexReal: public snPlotQwt {
public:
	snPlotComplexReal(std::string ptitle, snPlot::YAxisType py_axis_type,
			snCurveComplex *pc);
	virtual ~snPlotComplexReal();
	QWidget* getWidget();
	void replot();
	void addCurve(snCurveComplex *pc);
protected:
	std::vector<snCurveComplex *> curves;
};

class snPlotComplexImag: public snPlotQwt {
public:
	snPlotComplexImag(std::string ptitle, snPlot::YAxisType py_axis_type,
			snCurveComplex *pc);
	virtual ~snPlotComplexImag();
	QWidget* getWidget();
	void replot();
	void addCurve(snCurveComplex *pc);
protected:
	std::vector<snCurveComplex *> curves;
};

class snPlotComplexRealImag: public snPlotQwt {
public:
	snPlotComplexRealImag(std::string ptitle, snPlot::YAxisType py_axis_type,
			snCurveComplex *pc);
	virtual ~snPlotComplexRealImag();
	QWidget* getWidget();
	void replot();
	void addCurve(snCurveComplex *pc);
protected:
	std::vector<snCurveComplex *> curves;
};

class snPlotComplexMagn: public snPlotComplexReal {
public:
	snPlotComplexMagn(std::string ptitle, snPlot::YAxisType py_axis_type,
			snCurveComplex *pc);
	virtual ~snPlotComplexMagn();
	void replot();
};

class snPlotComplexAngle: public snPlotComplexImag {
public:
	snPlotComplexAngle(std::string ptitle, snPlot::YAxisType py_axis_type,
			snCurveComplex *pc);
	virtual ~snPlotComplexAngle();
	void replot();
};

class snPlotMagn: public snPlotQwt {
public:
	snPlotMagn(std::string ptitle, snPlot::YAxisType py_axis_type,
			snCurveComplex *pc, double _fsmpl);
	virtual ~snPlotMagn();
	QWidget* getWidget();
	void replot();
	void addCurve(snCurveComplex *pc);
private:
	std::vector<snCurveComplex *> curves;
	double fsmpl;
	double deltaf; // spectral resolution
	double freqmin, freqmax;
};

class snPlotAngl: public snPlotQwt {
public:
	snPlotAngl(std::string ptitle, snPlot::YAxisType py_axis_type,
			snCurveComplex *pc, double _fsmpl);
	virtual ~snPlotAngl();
	QWidget* getWidget();
	void replot();
	void addCurve(snCurveComplex *pc);
private:
	double fsmpl;
	double deltaf;
	double freqmin, freqmax;
	std::vector<snCurveComplex *> curves;
};

class snPlotSurface: public snPlotQwt3D {
public:
	snPlotSurface(std::string ptitle, snPlot::YAxisType py_axis_type,
			snCurve3D *pc);
	virtual ~snPlotSurface();
	QWidget* getWidget();
	void replot();
private:
	snCurve3D *curve;
};

#endif /* SNPLOT_H_ */
