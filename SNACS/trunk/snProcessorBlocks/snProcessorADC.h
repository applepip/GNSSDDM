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
 * \file snProcessorADC.h
 *
 * \author Frank Schubert
 */

#ifndef DEF_snProcessorADC_H_
#define DEF_snProcessorADC_H_ 1

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <complex>

#include "../snWidget/snWidget.h"
#include "../snSimulation/snBlock.h"
#include "../snSimulation/ConcurrentBuffer.h"

/**
 * \brief Class for analog to digital conversion (ADC) of a signal.
 *
 * \ingroup procBlocks
 */
class snProcessorADC: public snBlock {

public:
	/**
	 * \brief constructor
	 */
	snProcessorADC(libconfig::Setting &Settings, SNSignal pSNSignal, snWidget *psnw, ConcBuf *pConcBuf1,
			ConcBuf *pConcBufOut);
	~snProcessorADC();

	void run();
private:
	std::complex<double> *BufIn, *BufOut;

	double intermed_freq;
	bool adc_enable;
	int adc_bits, adc_states;
	bool vga_enable;
	double adc_divisor, adc_highest_value, adc_lowest_value;
	double vga_lowest_amplification_dB, vga_highest_amplification_dB;
	double vga_lowest_amplification, vga_highest_amplification;
	snCurveReal *vga_ampl_curve;
	snCurveReal *adc_hist_curve;

	// for plotting:
	snWidget *snw;
	snCurveComplex *curve;

	ConcBuf *ConcBuf1, *ConcBufOut;
	int PlotBufSize;
};
#endif
