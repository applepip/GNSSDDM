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

#include "snPlotConsole.h"

snPlotConsole::snPlotConsole(std::string ptitle, YAxisType py_axis_type) :
		snPlot(ptitle, py_axis_type) {

}

snPlotConsole::~snPlotConsole() {
}

void snPlotConsole::setXAxis(double xmin, double xmax) {

}

void snPlotConsole::setYAxis(double ymin, double ymax) {

}

void snPlotConsole::setXAxisLabel(std::string l) {

}

void snPlotConsole::setYAxisLabel(std::string l) {

}

void snPlotConsole::enableLegend() {

}

//snPlotRealConsole::snPlotRealConsole(std::string ptitle, snPlotConsole::YAxisType py_axis_type, snCurveReal *_curve) :
//		snPlotConsole(ptitle, py_axis_type) {
//
////	addCurve(_curve);
//}
//
//snPlotRealConsole::~snPlotRealConsole() {
//
//}
//
//void snPlotRealConsole::addCurve(snCurveReal *pc) {
////	curves.push_back(pc);
//}
//
//void snPlotRealConsole::replot() {
//
//}
//
//snPlotComplexConsole::snPlotComplexConsole(std::string ptitle, snPlotConsole::YAxisType py_axis_type,
//		snCurveComplex *_curve) :
//		snPlotConsole(ptitle, py_axis_type) {
//
////	addCurve(_curve);
//}
//
//snPlotComplexConsole::~snPlotComplexConsole() {
//
//}
//
//void snPlotComplexConsole::addCurve(snCurveComplex *pc) {
////	curves.push_back(pc);
//}
//
//void snPlotComplexConsole::replot() {
//
//}
//
//snPlotSurfaceConsole::snPlotSurfaceConsole(std::string ptitle, snPlot::YAxisType py_axis_type, snCurve3D *pc) :
//		snPlot(ptitle, py_axis_type), curve(pc) {
//
//}
//
//snPlotSurfaceConsole::~snPlotSurfaceConsole() {
//
//}
//
//void snPlotSurfaceConsole::replot() {
//
//}
//
//void snPlotSurfaceConsole::addCurve(snCurve3D *_curve) {
//
//}
