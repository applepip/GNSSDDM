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

#ifndef DEF_snSIGNALOUTPUTWIDGET_H_
#define DEF_snSIGNALOUTPUTWIDGET_H_ 1

#include "../snSimulation/snGlobal.h"
#include "../snConsumingModule.h"
#include "../snWidget/snWidget.h"

/**
 * \brief This class simply outputs a signal of a certain length to a widget window.
 *
 * \ingroup sinkBlocks
 */
class snSignalOutputWidget: public snConsumingModule {

public:
	snSignalOutputWidget(libconfig::Setting &Settings, SNSignal pSNSignal, widget_factory_base *widget_factory,
			ConcBuf3* pConcBuf1);

	void run();

	double get_total_simulation_time() {
		return circbuf->get_signal_length();
	}

private:
	// for plotting:
	std::complex<double> *VisBuf;
	unsigned int nBufs; // number of Bufs that should be plotted
	// for plotting:
	snWidget *snw;
	snCurveComplex *curve, *curve_magn;
	snCurveReal *curve_abs, *curve_phase;
};
#endif
