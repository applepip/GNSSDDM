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

#include "snPlot.h"

snPlot::snPlot(std::string ptitle, YAxisType py_axis_type, double _smpl_freq) :
		title(ptitle), y_axis_type(py_axis_type), smpl_freq(_smpl_freq) {
}

snPlot::~snPlot() {

}

//snPlot3D::snPlot3D(std::string ptitle, YAxisType py_axis_type) :
//		snPlot(ptitle, py_axis_type) {
//
//}
//
//snPlot3D::~snPlot3D() {
//
//}

//snPlotSurface::snPlotSurface(std::string ptitle, snPlot::YAxisType py_axis_type) :
//		snPlot(ptitle, py_axis_type) {
//}
//
//snPlotSurface::~snPlotSurface() {
//}
