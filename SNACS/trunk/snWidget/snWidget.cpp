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

#include "snWidget.h"


/**
 * \brief snWidget constructor.
 * @param parent
 * @param ptit
 */
snWidget::snWidget(QWidget *parent, QString ptit, WindowType ptype) :
	QWidget(parent), tit(ptit), type(ptype) {

	this->setWindowTitle(tit);
	m = new QMutex;

	m->lock();
	fVis = 0;
	m->unlock();
}

snWidget::~snWidget() {
	delete m;
}

int snWidget::requestPlotFlag() {
	m->lock();
	if (fVis) {
		return 1;
	} else {
		m->unlock();
		return 0;
	}
}

void snWidget::pushData() {
	m->lock();
	fVis = 1;
}

void snWidget::releasePlotFlag() {
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

void snWidget::replotWidgets() {
	m->lock();
	if (fVis == 0) {
		for (size_t i = 0; i < plots.size(); i++) {
			plots.at(i)->replot();
		}
		fVis = 1;
	}
	m->unlock();
}

void snWidget::addPlot(snPlot *plot) {
	plots.push_back(plot);

	if (this->layout() != 0)
		delete this->layout();

	QVBoxLayout *layout = new QVBoxLayout;
	for (size_t i = 0; i < plots.size(); i++) {
		layout->addWidget(plots.at(i)->getWidget());
	}
	this->setLayout(layout);
}
