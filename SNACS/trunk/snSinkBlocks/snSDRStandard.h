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
 * \file snSDR.h
 *
 * Software Defined Receiver block include file.
 *
 * \author Frank Schubert
 * \date 2008-08-25
 * \version 0.1
 */

#ifndef DEF_snSDRSTANDARD_H_
#define DEF_snSDRSTANDARD_H_ 1

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <complex>
#include <fstream>
#include <cmath>
#include <limits>

#include "../snSimulation/snSimulation.h"
#include "../snSinkBlocks/snSDR.h"
#include "../snWidget/snWidget.h"
#include "../snSimulation/ConcurrentBuffer.h"
#include "../snGNSS/snGNSSSignal.h"

/**
 * \brief Software Defined Receiver
 *
 * Contains the SDR of SNACS. Aquisition and tracking are performed.
 *
 * \ingroup sinkBlocks
 */
class snSDRStandard: public snSDR {

public:
	snSDRStandard(libconfig::Setting &Settings, SNSignal pSNSignal, ProcessingParameters &pProcSig,
			snWidget *psnw, snWidget *ptrkw, snWidget *ptrkw4, snWidget *ptrkw3, snWidget *ptrkw2,
			ConcBuf* pConcBuf1);
	~snSDRStandard();

	void run(); ///< implements QThread's run()

	bool dbl_eq(double x, double v) {
		// TODO
		return (std::fabs(x - v) <= 30.0 * std::numeric_limits<double>::epsilon());
	}

private:

	void setup_plots();

	//	int Aquisition(int aq_prn, double &AqFreq, Samples_Type &AqCodePhase);
	double cn0_estimation(const std::vector<double> &vI_P, const std::vector<double> &vQ_P);

	static double correlation(const std::vector<double> &a, const std::vector<double> &b);
	static std::vector<double> build_code(const std::vector<double> &indices, snModulation *m);

	/*
	 * \param shift in chips
	 */
	std::vector<double> build_shifted_code(double shift, snModulation *m, Samples_Type samples);
	double determine_gain_of_code(snModulation *m, double el_spacing);

	ProcessingParameters &ProcSig;

	std::ofstream oResult;

	bool I_enable, Q_enable;
	snGNSSSignal *gnss_signal;
	int prn;
	snGNSSCode *gnss_code_I, *gnss_code_Q;
	snModulation *modulation_I, *modulation_Q;

	int correlation_length_factor;
	double correlation_length;

	long aq_samples_number; ///< amount of samples for one aq block

	double aq_frequency;
	Samples_Type aq_code_phase;

	// Tracking
	int aPlCode;

	// for plotting:
	snCurve3D *curve_aq;
	snCurveComplex *curve_corr_e_I, *curve_corr_p_I, *curve_corr_l_I;
	snPlotComplexRealImag *plot_corrs_I;

	snCurveComplex *curve_corr_e_Q, *curve_corr_p_Q, *curve_corr_l_Q;

	snCurveReal *curve_incoming;
	snCurveComplex *curve_carrier_replica;
	snCurveComplex *curve_downmixed;

	snCurveReal *curve_code_e_I, *curve_code_p_I, *curve_code_l_I;
	snCurveReal *curve_code_e_Q, *curve_code_p_Q, *curve_code_l_Q;
	snCurveReal *curve_pll_raw_I, *curve_pll_raw_Q, *curve_pll_filtered, *curve_dll_raw_I,
			*curve_dll_raw_Q, *curve_dll_raw_IQ, *curve_dll_filtered, *curve_remcodephase,
			*curve_pr;
	snCurveReal *curve_corrfunc1_I, *curve_corrfunc2_I;
	snCurveReal *curve_corrfunc1_Q, *curve_corrfunc2_Q;
	snPlotReal *plot_pr;
	snPlotReal *plot_corrfunc1, *plot_corrfunc2;

	snCurveReal *curve_cn0;

	snWidget *snw, *trkw, *trkw2, *trkw3, *trkw4;

	ConcBuf *ConcBuf1;
	int PlotBufSize;
};

#endif
