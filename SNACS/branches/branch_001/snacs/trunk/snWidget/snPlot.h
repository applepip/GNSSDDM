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
#include <string>

#include "snCurve.h"

class snPlot {
public:
	enum YAxisType {
		Linear, Log20, Log10, FFTMagnitudeDB, FFTAngle, Angle
	};

	snPlot(std::string ptitle, YAxisType py_axis_type, double _smpl_freq = 0);
	virtual ~snPlot();
	virtual void replot() = 0;
	virtual void setYAxis(double ymin, double ymax) = 0;
	virtual void setXAxis(double xmin, double xmax) = 0;
	virtual void setXAxisLabel(std::string l) = 0;
	virtual void setYAxisLabel(std::string l) = 0;
	virtual void enableGrid() = 0;
	virtual void enableLegend() = 0;
	virtual void addCurve(snCurveReal *pc) = 0;
	virtual void addCurve(snCurveComplex *pc) = 0;
	virtual void addCurve(snCurve3D *pc) = 0;

protected:
	std::string title;
	YAxisType y_axis_type;
	double smpl_freq;
};

//class snPlot3D: public snPlot {
//public:
//	snPlot3D(std::string ptitle, YAxisType py_axis_type);
//	virtual ~snPlot3D();
//protected:
//	snCurve3D *curve;
//};

//class snPlotSurface: public snPlot {
//public:
//	snPlotSurface(std::string ptitle, snPlot::YAxisType py_axis_type);
//	virtual ~snPlotSurface();
//};

#endif /* SNPLOT_H_ */
