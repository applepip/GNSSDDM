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

#include <QWidget>
#include <QMutex>
#include <QVBoxLayout>

#include "snCurve.h"
#include "snPlot.h"

/**
 * \brief Widget class that generates time and frequency domain plots and 3D plots for the snBlocks.
 *
 * Since threads must never access widgets that were created in the main thread, snWidget serves as agent for plotting to windows created by the main thread. Threads can request time and frequency domain plots or 3D plots from it and main() can request snWidget to replot all plots. The snBlocks directly write to snWidget's member data to spped up the plotting process.
 * A snBlock can request the following plot types from snWidget:
 * - TPlots: Plots for time and frequency domain plots. snBlock writes data to snWidget->X and Y.
 * 	- TIPlot: plot inphase component
 * 	- TQPlot: plot quadrature component
 * 	- TIQPlot: plot inphase and quadrature component into one plot
 * 	- TSMPlot: plot spectrum magnitude (SM) component
 * 	- TSAPlot: plot spectrum angle (SA) component
 * - SPlot: Plot 3D plot. snBlock writes data to snWidget->SXY.
 * - XPlot:
 * - TLabel: Provides simple text label.
 */
class snWidget: public QWidget {

public:
//	enum PlotTypes {
	//	TIPlot, TQPlot, TIQPlot, TSMPlot, TSAPlot, TLabel, SPlot, XPlot
	//}; ///< different types of plots.
	enum WindowType {
		INLINE, SEPARATE
	};

	snWidget(QWidget *parent, QString ptit, WindowType ptype);
	~snWidget();

	QString getTitle() const {
		return tit;
	};

	WindowType getWindowType() {
		return type;
	}
	;

	//	virtual void newPlot(PlotTypes, YTypes, std::string);
	//	virtual void replot_widgets();

	/**
	 * \brief called by main() to redraw all plots.
	 */
	void replotWidgets();

	/**
	 * \brief has to be called befor a snBlock can write new plot data.
	 * @return true: write new data. false: don't write
	 */
	int requestPlotFlag();

	/**
	 * \brief Push mode for data. snBlock writes data to snWidget.
	 */
	void pushData();

	/**
	 * \brief has to be called after a snBlock wrote new plot data to snWidget.
	 */
	void releasePlotFlag();

	void addPlot(snPlot *plot);

private:
	QString tit;
	WindowType type;

	int fCurveStyle;

	int PlotXSize;

	int fVis; ///< set to 1 if new plots are requested
	QMutex *m; ///< Mutex to protect fVis
protected:
	std::vector<snPlot *> plots;
};

#endif
