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

#ifndef DEF_snProcessorLPF_H_
#define DEF_snProcessorLPF_H_ 1

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <complex>

#include <QMutex>
#include <QThread>

#include "../snWidget/snWidget.h"
#include "../snSimulation/snBlock.h"
#include "../snSimulation/ConcurrentBuffer.h"

#include "../snSimulation/snCmplxFIRFilter.h"
//#include "../snSimulation/snFIRFilter.h"


/**
 * \brief Class for low pass filtering.
 *
 * \ingroup procBlocks
 *
 */
class snProcessorLPF: public snBlock {

public:
	snProcessorLPF(libconfig::Setting &Settings, SNSignal pSNSignal, ProcessingParameters &ProcSig, snWidget *psnw, ConcBuf* pConcBuf1, ConcBuf* pConcBuf2);
	~snProcessorLPF();

	void run();

	static std::vector<double> remez_coefficients(double stop, size_t numtaps);
private:
	size_t numtaps;
	int filter_order;
	double cut_off_frequency;

	std::vector<std::complex<double> > Coeffs;
	snCmplxFIRFilter *CmplxFilter;

	// for plotting:
	snWidget *snw;
	snCurveComplex *curve;

	ConcBuf *ConcBuf1, *ConcBuf2;
};

#endif
