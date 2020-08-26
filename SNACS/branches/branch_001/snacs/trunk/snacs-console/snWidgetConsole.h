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

#ifndef DEF_snWidgetConsole
#define DEF_snWidgetConsole 1

#include <QMutex>
#include <iostream>

#include "../snSimulation/snSimulation.h"
#include "../snWidget/snWidget.h"
#include "snPlotConsole.h"

class snWidgetConsole: public snWidget {

public:
	snWidgetConsole(std::string ptit, WindowType ptype);
	~snWidgetConsole();

	void replotWidgets();
	int requestPlotFlag();
	void pushData();
	void releasePlotFlag();
	void addPlot(snPlot *plot);

//	QWidget *get_widget() {
//		return theWidget;
//	}

//	snCurveReal *get_new_curve_real(int pn, snCurve::CurveColor pcolorI);
//	snCurveComplex *get_new_curve_complex(int pn, snCurve::CurveColor pcolorI, snCurve::CurveColor pcolorQ);
	snPlot *get_new_plot_real(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveReal *_curve);
	snPlot *get_new_plot_complex(std::string ptitle, snPlot::YAxisType py_axis_type, snCurveComplex *_curve);
	snPlot *get_new_plot_surface(std::string ptitle, snPlot::YAxisType py_axis_type, snCurve3D *_curve);
private:
	std::vector<snPlotConsole *> plots;
//	int fVis; ///< set to 1 if new plots are requested
	QMutex *m; ///< Mutex to protect fVis
};

//typedef std::tr1::shared_ptr<snWidgetConsole> snWidgetConsolePtr;

class console_widget_factory: public widget_factory_base {
public:
	virtual snWidget *get_new_widget(std::string name) {
//		snWidgetConsolePtr widget(new snWidgetConsole(name, snWidgetConsole::INLINE));
//		snWs.push_back(widget);
//		return widget.get();
		return new snWidgetConsole(name, snWidgetConsole::INLINE);
	}

//	std::vector<snWidgetConsolePtr> get_created_widgets() {
//		return snWs;
//	}

private:
//	std::vector<snWidgetConsolePtr> snWs; ///< contains pointers to all widgets
};

#endif
