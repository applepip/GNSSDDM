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
 * \file snProcessorChannel.h
 *
 * \author Frank Schubert
 */

#ifndef DEF_SNPROCCHANNEL_H_
#define DEF_SNPROCCHANNEL_H_ 1

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <complex>
#include <cmath>

#include "../snWidget/snWidget.h"
#include "../snSimulation/snBlock.h"
#include "../snSimulation/ConcurrentBuffer.h"
#include "../snSimulation/snReadCIRFile.h"
#include "../snSimulation/snCmplxFIRFilter.h"

/**
 * \brief snProcessorChannel implements convolution of a base band signal through a linear channel through FIR filtering.
 *
 * \ingroup procBlocks
 *
 */
class snProcessorChannel: public snBlock {

public:
	enum interpolation_types {
		SINC, RRC
	};

	snProcessorChannel(libconfig::Setting &Settings, SNSignal pSNSignal,
			ProcessingParameters &ProcSig, snWidget *psnwCIR, snWidget *psnwFIR, snWidget *psnw,
			ConcBuf* pConcBuf1, ConcBuf* pConcBuf2);
	~snProcessorChannel();

	void run();

private:
	interpolation_types interpolation_type;
	double rrc_rolloff_factor;

	snReadCIRFile *file;

	// interpolator:
	double add_delay;
	Blocks_Type aCIR;
	double CIRrate; // CIR rate

	double interpolation_bandwidth;
	double MaxDelay;//, MaxPR ;

	int aFIR;

	// for plotting:
	snWidget *cir_snw, *fir_snw, *out_snw;
	snCurveComplex *fir_curve;
	snCurveComplex *cir_curve;
	snCurveComplex *out_curve;

	ConcBuf *ConcBuf1, *ConcBuf2;
};

#endif
