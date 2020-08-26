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

#ifndef DEF_snSignalGenerate_H_
#define DEF_snSignalGenerate_H_ 1

#include <fstream>
#include <cstdlib>
#include <iostream>

#include "../snWidget/snWidget.h"
#include "../snSimulation/ConcurrentBuffer.h"
#include "../snGNSS/snGNSSSignal.h"

#include "../snSimulation/snBlock.h"

/**
 * \brief This class generates sampled GPS base band signals.
 *
 * \ingroup sourceBlocks
 */
class snSignalGenerate: public snBlock {

public:
	snSignalGenerate(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *psnw, ConcBuf* pConcBuf1);

	~snSignalGenerate();

	void run();

private:
	//unsigned int plot_x_max;
	snGNSSSignal *gnss_signal;
	bool inphase_enabled, quadrature_enabled;

	int prn;
	snModulation *inphase_modulation, *quadrature_modulation;

	double start_sample; ///< sample at which the signal generation should start. output zeros before.

	// for plotting:
	snWidget *snw;
	snCurveComplex *curve;

	ConcBuf *ConcBuf1;
};

#endif
