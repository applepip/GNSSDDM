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

#include "../snSimulation/snSimulation.h"
#include "../snSimulation/snBlock.h"
#include "../snWidget/snWidget.h"
#include "../snSimulation/ConcurrentBuffer.h"

/**
 * \brief This class samples and mixes a signal and outputs it to a file for processing with extern software receivers, e.g. in Matlab.
 *
 * \ingroup sinkBlocks
 */
class snSignalOutputBinary: public snBlock {
public:
	enum output_type {
		SIGNED_CHAR, DOUBLE
	};

	snSignalOutputBinary(libconfig::Setting &Settings, SNSignal pSNSignal,
			snWidget *psnw, ConcBuf* pConcBuf1);
	~snSignalOutputBinary();

	void run();
private:
	output_type ot;

	std::fstream *oFile;

	// for plotting:
	snWidget *snw;
	snCurveReal *curve;

	ConcBuf *ConcBuf1;
	int PlotBufSize, BufSize;
};
#endif
