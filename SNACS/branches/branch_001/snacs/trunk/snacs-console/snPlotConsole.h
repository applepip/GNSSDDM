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
 * snPlotConsole.h
 *
 *      Author: Frank Schubert
 */

#ifndef snPlotConsole_H_
#define snPlotConsole_H_

#include <iostream>
#include <tr1/memory>

#include "../snWidget/snPlot.h"

/*
 * Implements a plotting class using the Console widget set for Qt.
 */
class snPlotConsole: public snPlot {
public:
	snPlotConsole(std::string ptitle, YAxisType py_axis_type);
	virtual ~snPlotConsole();
//	QWidget* getWidget() {
//		return plot.get();
//	}

	void replot() {
	}

	void addCurve(snCurveReal *pc) {

	}
	void addCurve(snCurveComplex *pc) {

	}
	void addCurve(snCurve3D *pc) {

	}

	void setYAxis(double ymin, double ymax);
	void setXAxis(double xmin, double xmax);
	void setXAxisLabel(std::string l);
	void setYAxisLabel(std::string l);

	void enableGrid() {

	}

	void enableLegend();
};

///*
// * Implements a plotting class for real data
// */
//class snPlotRealConsole: public snPlotConsole {
//public:
//	snPlotRealConsole(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveReal *_curve);
//	virtual ~snPlotRealConsole();
//	void replot();
//	void addCurve(snCurve *pc);
//	void addCurve(snCurveComplex *pc) {
//		std::cerr << "error: snPlotRealConsole::addCurve cannot process snCurveComplex for " << pc->get_name()
//				<< "\n";
//		std::abort();
//	}
//	void addCurve(snCurve3D *pc) {
//		std::cerr << "error: snPlotRealConsole::addCurve cannot process snCurve3D for " << pc->get_name() << "\n";
//		std::abort();
//	}
//
//protected:
//	std::vector<snCurveReal *> curves;
//};
//
///*
// * Implements a plotting class for complex data
// */
//class snPlotComplexConsole: public snPlotConsole {
//public:
//	enum DataType {
//		real, imag
//	};
//
//	snPlotComplexConsole(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveComplex *_curve);
//	virtual ~snPlotComplexConsole();
//
//	void replot();
//	void addCurve(snCurveComplex *pc);
//	void addCurve(snCurveReal *pc) {
//		std::cerr << "error: snPlotComplexConsole::addCurve cannot process snCurveReal for " << pc->get_name()
//				<< "\n";
//		std::abort();
//	}
//	void addCurve(snCurve3D *pc) {
//		std::cerr << "error: snPlotComplexConsole::addCurve cannot process snCurve3D for " << pc->get_name()
//				<< "\n";
//		std::abort();
//	}
//protected:
//	std::vector<snCurveComplex *> curves;
//	std::vector<DataType> data_types;
//	double smpl_freq;
//};

///*
// * Implements a plotting class for 3d data
// */
//class snPlotSurfaceConsole: public snPlot {
//public:
//	snPlotSurfaceConsole(std::string ptitle, snPlot::YAxisType py_axis_type, snCurve3D *pc);
//	virtual ~snPlotSurfaceConsole();
//	void replot();
//
//	void setYAxis(double ymin, double ymax) {
//	}
//	void setXAxis(double xmin, double xmax) {
//	}
//	void setXAxisLabel(std::string l) {
//	}
//	void setYAxisLabel(std::string l) {
//	}
//	void enableGrid() {
//	}
//	void enableLegend() {
//	}
//	void addCurve(snCurveReal *pc) {
//		std::cerr << "error: snPlotSurfaceConsole::addCurve cannot process snCurveReal for " << pc->get_name()
//				<< "\n";
//		std::abort();
//	}
//	void addCurve(snCurveComplex *pc) {
//		std::cerr << "error: snPlotSurfaceConsole::addCurve cannot process snCurveComplex for " << pc->get_name()
//				<< "\n";
//		std::abort();
//	}
//	void addCurve(snCurve3D *_curve);
//
//private:
////	std::tr1::shared_ptr<ConsoleSurfacePlot> plot;
//	std::tr1::shared_ptr<snCurve3D> curve;
//};

#endif
