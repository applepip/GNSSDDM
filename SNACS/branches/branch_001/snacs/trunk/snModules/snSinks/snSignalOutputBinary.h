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

#ifndef DEF_snSIGNALOUTPUTBINARY_H_
#define DEF_snSIGNALOUTPUTBINARY_H_ 1

#include "../snSimulation/snGlobal.h"
#include "../snSimulation/snSerialModule.h"
#include "../snWidget/snWidget.h"

/**
 * \brief This class samples and mixes a signal and outputs it to a file for processing with extern software receivers, e.g. in Matlab.
 *
 * \ingroup sinkBlocks
 */
class snSignalOutputBinary: public snSerialModule {
public:
	enum output_type {
		SIGNED_CHAR, REAL_DOUBLE, COMPLEX_DOUBLE
	};

			snSignalOutputBinary(libconfig::Setting &Settings, SNSignal pSNSignal, ProcessingParameters &_ProcSig, snWidget *psnw,
					ConcBuf3* _circbuf);
	~snSignalOutputBinary();

	double get_total_simulation_time() {
		return circbuf->get_signal_length();
	}

	void run();
private:
	output_type ot;

	std::fstream *oFile;

	ProcessingParameters &ProcSig;

	// for plotting:
	snWidget *snw;
	snCurveComplex *curve;

	unsigned int nBufs; // number of Bufs that should be plotted
};
#endif
