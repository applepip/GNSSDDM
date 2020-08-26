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

#include <cstdlib>
#include "snWidgetConsole.h"

snWidgetConsole::snWidgetConsole(std::string ptit, WindowType ptype) :
		snWidget(ptit, ptype) {

	m = new QMutex;
//
//	m->lock();
//	fVis = 0;
//	m->unlock();
}

snWidgetConsole::~snWidgetConsole() {
	for (size_t k = 0; k < plots.size(); k++)
		delete plots.at(k);

	delete m;
//	delete theWidget;
}

int snWidgetConsole::requestPlotFlag() {
//	m->lock();
//	if (fVis) {
//		return 1;
//	} else {
//		m->unlock();
//		return 0;
//	}
	return 0;
}

void snWidgetConsole::pushData() {
//	m->lock();
//	fVis = 1;
}

void snWidgetConsole::releasePlotFlag() {
//	// m must be locked at this point. if it is not: error
//	if (m->tryLock() != false) {
//		std::cout
//				<< "\n snWidget::releasePlotFlag(): m must be locked at this point, but it is in unlocked stated. exit.\n";
//		std::exit(1);
//	}
//	// ok, m is locked.
//	fVis = 0;
//	m->unlock();
}

void snWidgetConsole::replotWidgets() {
//	m->lock();
//	if (fVis == 0) {
//		for (size_t i = 0; i < plots.size(); i++) {
//			plots.at(i)->replot();
//		}
//		fVis = 1;
//	}
//	m->unlock();
}

void snWidgetConsole::addPlot(snPlot *plot) {
//	plots.push_back(static_cast<snPlotConsole *>(plot));

}

snPlot *snWidgetConsole::get_new_plot_real(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurveReal *_curve) {
	return new snPlotConsole(ptitle, py_axis_type);
}

snPlot *snWidgetConsole::get_new_plot_complex(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurveComplex *_curve) {
	return new snPlotConsole(ptitle, py_axis_type);
}

snPlot *snWidgetConsole::get_new_plot_surface(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurve3D *_curve) {
	return new snPlotConsole(ptitle, py_axis_type);
}
