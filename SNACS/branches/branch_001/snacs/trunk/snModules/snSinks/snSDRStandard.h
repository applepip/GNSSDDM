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

#include "../snSimulation/snGlobal.h"
#include "../snModules/snSinks/snSDR.h"
#include "../snWidget/snWidget.h"
#include "../snSimulation/ConcurrentBuffer3.h"
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
			widget_factory_base *widget_factory, ConcBuf3* _circ_buf);
	~snSDRStandard();

	void run(); ///< implements QThread's run()

	bool dbl_eq(double x, double v) {
		// TODO
		return (std::fabs(x - v) <= 30.0 * std::numeric_limits<double>::epsilon());
	}
	double get_total_simulation_time() {
		return circbuf->get_signal_length();
	}
private:

	void setup_plots();

	//	int Aquisition(int aq_prn, double &AqFreq, Samples_Type &AqCodePhase);
	//double cn0_estimation(const itpp::vec &vI_P, const itpp::vec &vQ_P);

	static double correlation(const itpp::vec &a, const itpp::vec &b);
	static itpp::vec build_code(const itpp::vec &indices, snModulation *m);

	/*
	 * \param shift in chips
	 */
	itpp::vec build_shifted_code(double shift, snModulation *m, Samples_Type samples);
	double determine_gain_of_code(snModulation *m, double el_spacing);

	ProcessingParameters &ProcSig;

	Blocks_Type cur_block;

	std::ofstream oResult;

	bool enable_downmixing; ///< if true, intermediate frequency is > 0 => enable mixing down the signal to baseband

	bool I_enable, Q_enable;
	snGNSSSignal *gnss_signal;
	int prn;
	snGNSSCode *gnss_code_I, *gnss_code_Q;
	snModulation *modulation_I, *modulation_Q;

	size_t correlation_length_factor;
	double correlation_length;
	int expected_corr_intervals; ///< the expected number of correlation intervals

	long aq_samples_number; ///< amount of samples for one aq block

	double aq_frequency;
	Samples_Type aq_code_phase;

	// Tracking
	int aPlCode;

	// datasets to write results continuously:
	DataSet *ds_pll_raw, *ds_pll_filtered, *ds_dll_raw, *ds_dll_filtered, *ds_remcodephase, *ds_pseudoranges,
			*ds_corr_early, *ds_corr_prompt, *ds_corr_late, *ds_c_n0;

	// for plotting:
	snCurve3D *curve_aq;
	snCurveComplex *curve_corr_e_I, *curve_corr_p_I, *curve_corr_l_I;
	snCurveComplex *curve_corr_e_Q, *curve_corr_p_Q, *curve_corr_l_Q;
	snPlot *plot_corrs_I;

	snCurveComplex *curve_incoming;
	snCurveComplex *curve_carrier_replica;
	snCurveComplex *curve_downmixed;

	snCurveReal *curve_code_e_I, *curve_code_p_I, *curve_code_l_I;
	snCurveReal *curve_code_e_Q, *curve_code_p_Q, *curve_code_l_Q;
	snCurveReal *curve_pll_raw_I, *curve_pll_raw_Q, *curve_pll_filtered, *curve_dll_raw_I, *curve_dll_raw_Q,
			*curve_dll_raw_IQ, *curve_dll_filtered, *curve_remcodephase, *curve_pr;
	snCurveReal *curve_corrfunc1_I, *curve_corrfunc2_I;
	snCurveReal *curve_corrfunc1_Q, *curve_corrfunc2_Q;
	snPlot *plot_pr;
	snPlot *plot_corrfunc1, *plot_corrfunc2;

	snCurveReal *curve_cn0, *curve_cn0_jost, *curve_cn0_mm, *curve_cn0_bl;

	snWidget *snw, *trkw, *trkw2, *trkw3, *trkw4;

	int PlotBufSize;
};

#endif
