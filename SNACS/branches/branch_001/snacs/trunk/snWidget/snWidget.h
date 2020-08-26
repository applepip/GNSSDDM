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

/**
 * \file snWidget/snWidget.h
 *
 * snWidget class file header.
 *
 * \author Frank Schubert
 * \date 2008-08-25
 * \version 0.1
 */
#ifndef DEF_SNWIDGET
#define DEF_SNWIDGET 1

#include <iostream>
#include <string>

#include "snCurve.h"
#include "snPlot.h"

/**
 * \brief Widget base class. snPlots can be added to snWidget.
 */
class snWidget {

public:
	enum WindowType {
		INLINE, SEPARATE
	};

	snWidget(std::string ptit, WindowType ptype);
	virtual ~snWidget();

	std::string getTitle() const {
		return tit;
	}

	WindowType getWindowType() {
		return type;
	}

	/**
	 * \brief called by main() to redraw all plots.
	 */
	virtual void replotWidgets() = 0;

	/**
	 * \brief has to be called befor a snBlock can write new plot data.
	 * @return true: write new data. false: don't write
	 */
	virtual int requestPlotFlag() = 0;

	/**
	 * \brief Push mode for data. snBlock writes data to snWidget.
	 */
	virtual void pushData() = 0;

	/**
	 * \brief has to be called after a snBlock wrote new plot data to snWidget.
	 */
	virtual void releasePlotFlag() = 0;

	virtual void addPlot(snPlot *plot) = 0;
	virtual snPlot *get_new_plot_real(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveReal *_curve) = 0;
	virtual snPlot *get_new_plot_complex(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveComplex *_curve) = 0;
	virtual snPlot *get_new_plot_surface(std::string ptitle, snPlot::YAxisType py_axis_type, snCurve3D *_curve) = 0;

private:
	std::string tit;
	WindowType type;
};

/**
 * Base class to provide a widget factory.
 * inspired by "functionioids", see http://www.parashift.com/c++-faq-lite/pointers-to-members.html
 */
class widget_factory_base {
public:
	//	widget_factory_functionoid();
	virtual ~widget_factory_base() = 0;

	virtual snWidget *get_new_widget(std::string name) = 0;
};

inline widget_factory_base::~widget_factory_base() { }

#endif
