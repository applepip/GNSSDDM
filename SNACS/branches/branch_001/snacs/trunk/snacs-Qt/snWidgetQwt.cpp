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

/**
 * \file snWidget.cpp
 *
 * Widget class which allows threads to plot into the main program window.
 *
 * \author Frank Schubert
 * \date 2008-08-25
 * \version 0.1
 */
#include <cstdlib>
#include "snWidgetQwt.h"

/**
 * \brief snWidget constructor.
 * @param parent
 * @param ptit
 */
snWidgetQwt::snWidgetQwt(QWidget *parent, std::string ptit, WindowType ptype) :
		snWidget(ptit, ptype), theWidget(new QWidget(parent)) {

	theWidget->setWindowTitle(getTitle().c_str());
	m = new QMutex;

	m->lock();
	fVis = 0;
	m->unlock();
}

snWidgetQwt::~snWidgetQwt() {
	for (size_t k = 0; k < plots.size(); k++)
		delete plots.at(k);

	delete m;
	delete theWidget;
}

int snWidgetQwt::requestPlotFlag() {
	m->lock();
	if (fVis) {
		return 1;
	} else {
		m->unlock();
		return 0;
	}
}

void snWidgetQwt::pushData() {
	m->lock();
	fVis = 1;
}

void snWidgetQwt::releasePlotFlag() {
	// m must be locked at this point. if it is not: error
	if (m->tryLock() != false) {
		std::cout
				<< "\n snWidget::releasePlotFlag(): m must be locked at this point, but it is in unlocked stated. exit.\n";
		std::exit(1);
	}
	// ok, m is locked.
	fVis = 0;
	m->unlock();
}

void snWidgetQwt::replotWidgets() {
	m->lock();
	if (fVis == 0) {
		for (size_t i = 0; i < plots.size(); i++) {
			plots.at(i)->replot();
		}
		fVis = 1;
	}
	m->unlock();
}

void snWidgetQwt::addPlot(snPlot *plot) {
//	snPlotQwtPtr new_plot(static_cast<snPlotQwt *>(plot));
//	plots.push_back(new_plot);

	plots.push_back(static_cast<snPlotQwt *>(plot));

	if (theWidget->layout() != 0)
		delete theWidget->layout();

	QVBoxLayout *layout = new QVBoxLayout;
	for (size_t i = 0; i < plots.size(); i++) {
		layout->addWidget(plots.at(i)->getWidget());
	}
	theWidget->setLayout(layout);
}

snPlot *snWidgetQwt::get_new_plot_real(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurveReal *_curve) {
	return new snPlotRealQwt(ptitle, py_axis_type, _curve);
}

snPlot *snWidgetQwt::get_new_plot_complex(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurveComplex *_curve) {
	return new snPlotComplexQwt(ptitle, py_axis_type, _curve);
}

snPlot *snWidgetQwt::get_new_plot_surface(std::string ptitle, snPlot::YAxisType py_axis_type,
		snCurve3D *_curve) {
	return new snPlotSurfaceQwt(ptitle, py_axis_type, _curve);
}
